--
-- LC display via i2c bus
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--
-- reference for i2c bus usage:
--	- https://www.digikey.com/eewiki/pages/viewpage.action?pageId=10125324
--

library ieee;
use ieee.std_logic_1164.all;
use work.conv.all;


entity lcd is
	generic(
		-- clock
		constant main_clk : natural := 50_000_000;

		-- i2c addresses of the LCD
		constant num_i2c_addrbits : natural := 8;
		constant num_i2c_databits : natural := 8;
		constant i2c_writeaddr : std_logic_vector(num_i2c_addrbits-1 downto 0) := x"10";
		constant i2c_readaddr : std_logic_vector(num_i2c_addrbits-1 downto 0) := x"11";

		-- number of commands in the LCD init ROM
		constant init_size : natural := 4;
		constant num_init_addrbits : natural := 2;
		constant num_init_databits : natural := num_i2c_databits;

		-- number of characters on the LCD
		constant lcd_size : natural := 4*20;
		constant num_lcd_addrbits : natural := 7;
		constant num_lcd_databits : natural := num_i2c_databits;

		-- start address of the display buffer in the RAM/ROM
		constant rom_start_addr : natural := 0
	);

	port(
		-- main clock
		in_clk : in std_logic;

		-- reset
		in_reset : in std_logic;
		-- reset for LCD
		out_lcd_reset : out std_logic;

		-- i2c bus
		in_i2c_busy, in_i2c_error : in std_logic;
		out_i2c_enable : out std_logic;
		out_i2c_addr : out std_logic_vector(num_i2c_addrbits-1 downto 0);
		out_i2c_data : out std_logic_vector(num_i2c_databits-1 downto 0);

		-- LCD init ROM
		out_initrom_addr : out std_logic_vector(num_init_addrbits-1 downto 0);
		in_initrom_word : in std_logic_vector(num_init_databits-1 downto 0);

		-- display buffer RAM/ROM
		out_rom_addr : out std_logic_vector(num_lcd_addrbits-1 downto 0);
		in_rom_word : in std_logic_vector(num_lcd_databits-1 downto 0)
	);
end entity;



architecture lcd_impl of lcd is
	-- states
	type t_lcd_state is (Wait_Reset, Reset, Resetted, ReadInitROM, Wait_ReadROM, ReadROM);
	signal lcd_state, next_lcd_state : t_lcd_state;
	
	signal lcd_reset, next_lcd_reset : std_logic;
	signal initrom_addr, next_initrom_addr : std_logic_vector(num_init_addrbits-1 downto 0);
	signal rom_addr, next_rom_addr : std_logic_vector(num_lcd_addrbits-1 downto 0);
	
	signal i2c_enable, next_i2c_enable : std_logic;
	signal i2c_addr, next_i2c_addr : std_logic_vector(num_i2c_addrbits-1 downto 0);
	signal i2c_data, next_i2c_data : std_logic_vector(num_i2c_databits-1 downto 0);
	signal i2c_last_busy, i2c_cycle : std_logic;

	-- delays
	constant wait_init : natural := main_clk/1000*10;	-- 10 ms
	constant wait_reseton : natural := main_clk/1000*1;	-- 1 ms
	constant wait_resetoff : natural := main_clk/1000*1;	-- 1 ms
	constant wait_preready : natural := main_clk/1000_000*250;	-- 250 us

	signal init_cycle, next_init_cycle : natural range 0 to init_size := 0;
	signal write_cycle, next_write_cycle : integer range -1 to lcd_size := -1;

	-- busy wait counter
	signal wait_counter_init : natural range 0 to wait_init := 0;
	signal wait_counter_init_finished : std_logic := '0';

	signal wait_counter_reseton : natural range 0 to wait_reseton := 0;
	signal wait_counter_reseton_finished : std_logic := '0';

	signal wait_counter_resetoff : natural range 0 to wait_resetoff := 0;
	signal wait_counter_resetoff_finished : std_logic := '0';

	signal wait_counter_preready : natural range 0 to wait_preready := 0;
	signal wait_counter_preready_finished : std_logic := '0';

	-- hack for ghdl, using local static values
	constant static_init_size : natural := 4; --init_size;
	constant static_lcd_size : natural := 4*20; --lcd_size;

	-- control byte: only data bytes following
	constant ctrl_onlydata : std_logic_vector(num_i2c_databits-1 downto 0) := "01000000";

begin

	-- outputs
	out_lcd_reset <= lcd_reset;
	out_i2c_enable <= i2c_enable;
	out_i2c_addr <= i2c_addr;
	out_i2c_data <= i2c_data;
	out_rom_addr <= rom_addr;
	out_initrom_addr <= initrom_addr;


	-- rising edge of i2c busy signal
	i2c_cycle <= (in_i2c_busy) and (not i2c_last_busy);


	proc_states : process(in_clk, in_reset) begin
		-- reset
		if in_reset = '0' then
			lcd_state <= Wait_Reset;
			lcd_reset <= '1';

			rom_addr <= (others=>'0');
			initrom_addr <= (others=>'0');

			i2c_enable <= '0';
			i2c_addr <= (others=>'0');
			i2c_data <= (others=>'0');
			i2c_last_busy <= '0';

			init_cycle <= 0;
			write_cycle <= -1;

		-- clock
		elsif rising_edge(in_clk) then
			lcd_state <= next_lcd_state;
			lcd_reset <= next_lcd_reset;

			rom_addr <= next_rom_addr;
			initrom_addr <= next_initrom_addr;

			i2c_enable <= next_i2c_enable;
			i2c_addr <= next_i2c_addr;
			i2c_data <= next_i2c_data;
			i2c_last_busy <= in_i2c_busy;

			init_cycle <= next_init_cycle;
			write_cycle <= next_write_cycle;
		end if;
	end process;
		

	-- process for busy wait timers
	proc_timer : process(in_clk, in_reset) begin
		-- reset
		if in_reset = '0' then
			wait_counter_init <= 0;
			wait_counter_reseton <= 0;
			wait_counter_resetoff <= 0;
			wait_counter_preready <= 0;

			wait_counter_init_finished <= '0';
			wait_counter_reseton_finished <= '0';
			wait_counter_resetoff_finished <= '0';
			wait_counter_preready_finished <= '0';

		elsif rising_edge(in_clk) then
			if next_lcd_state = Wait_Reset then
				if wait_counter_init = wait_init then
					wait_counter_init <= 0;
					wait_counter_init_finished <= '1';
				else
					wait_counter_init <= wait_counter_init + 1;
				end if;
			else
				wait_counter_init <= 0;
				wait_counter_init_finished <= '0';
			end if;

			if next_lcd_state = Reset then
				if wait_counter_reseton = wait_reseton then
					wait_counter_reseton <= 0;
					wait_counter_reseton_finished <= '1';
				else
					wait_counter_reseton <= wait_counter_reseton + 1;
				end if;
			else
				wait_counter_reseton <= 0;
				wait_counter_reseton_finished <= '0';
			end if;
			
			if next_lcd_state = Resetted then
				if wait_counter_resetoff = wait_resetoff then
					wait_counter_resetoff <= 0;
					wait_counter_resetoff_finished <= '1';
				else
					wait_counter_resetoff <= wait_counter_resetoff + 1;
				end if;
			else
				wait_counter_resetoff <= 0;
				wait_counter_resetoff_finished <= '0';
			end if;

			if next_lcd_state = Wait_ReadROM then
				if wait_counter_preready = wait_preready then
					wait_counter_preready <= 0;
					wait_counter_preready_finished <= '1';
				else
					wait_counter_preready <= wait_counter_preready + 1;
				end if;
			else
				wait_counter_preready <= 0;
				wait_counter_preready_finished <= '0';
			end if;

		end if;
	end process;

	
	proc_transitions : process(
		lcd_state, i2c_cycle, init_cycle, write_cycle, 
		lcd_reset, rom_addr, initrom_addr, in_initrom_word, in_rom_word,
		i2c_enable, i2c_addr, i2c_data, in_i2c_busy,
		wait_counter_init_finished, wait_counter_reseton_finished, wait_counter_resetoff_finished, wait_counter_preready_finished)
	begin
		-- defaults
		next_lcd_state <= lcd_state;
		next_lcd_reset <= lcd_reset;

		next_rom_addr <= rom_addr;
		next_initrom_addr <= initrom_addr;

		next_i2c_enable <= i2c_enable;
		next_i2c_addr <= i2c_addr;
		next_i2c_data <= i2c_data;
	
		next_init_cycle <= init_cycle;
		next_write_cycle <= write_cycle;


		-- fsm
		case lcd_state is

			when Wait_Reset =>	
				if wait_counter_init_finished= '1' then
					next_lcd_state <= Reset;
				end if;


			when Reset =>
				next_lcd_reset <= '0';
				
				-- wait
				if wait_counter_reseton_finished= '1' then
					next_lcd_state <= Resetted;
				end if;


			when Resetted => 
				next_lcd_reset <= '1';

				-- wait
				if wait_counter_resetoff_finished= '1' then
					next_lcd_state <= ReadInitROM;
				end if;


			when ReadInitROM =>
				if i2c_cycle='1' then
					next_init_cycle <= init_cycle + 1;
				end if;

				case init_cycle is
					-- sequence finished
					when static_init_size =>
						next_i2c_enable <= '0';
						if in_i2c_busy='0' then
							next_lcd_state <= Wait_ReadROM;
							next_init_cycle <= 0;
						end if;

					-- read from init ROM
					when others =>
						next_initrom_addr <= 
							nat_to_logvec(init_cycle, num_init_addrbits);
						next_i2c_addr <= i2c_writeaddr;
						next_i2c_data <= in_initrom_word;
						next_i2c_enable <= '1';
				end case;


			when Wait_ReadROM =>
				if wait_counter_preready_finished= '1' then
					next_lcd_state <= ReadROM;
				end if;


			when ReadROM =>	
				if i2c_cycle='1' then
					next_write_cycle <= write_cycle + 1;
				end if;

				case write_cycle is
					-- control byte
					when -1 =>
						next_i2c_addr <= i2c_writeaddr;
						next_i2c_data <= ctrl_onlydata;
						next_i2c_enable <= '1';

					-- sequence finished
					when static_lcd_size =>
						next_i2c_enable <= '0';
						if in_i2c_busy='0' then
							next_lcd_state <= Wait_ReadROM;
							next_write_cycle <= -1;
						end if;

					-- read characters from display buffer RAM/ROM
					when others =>
						next_rom_addr <=
							int_to_logvec(write_cycle + rom_start_addr, num_lcd_addrbits);
						next_i2c_addr <= i2c_writeaddr;
						next_i2c_data <= in_rom_word;
						next_i2c_enable <= '1';
				end case;


			when others =>
				next_lcd_state <= Wait_Reset;
		end case;
	end process;

end architecture;
