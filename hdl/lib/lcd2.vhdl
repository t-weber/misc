--
-- LC display via i2c bus (using a fixed init sequence)
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

		-- display buffer RAM/ROM
		out_rom_addr : out std_logic_vector(num_lcd_addrbits-1 downto 0);
		in_rom_word : in std_logic_vector(num_lcd_databits-1 downto 0)
	);
end entity;



architecture lcd_impl of lcd is
	-- states
	type t_lcd_state is (Wait_Reset, Reset, Resetted, ReadInitSeq, Wait_ReadROM, ReadROM);
	signal lcd_state, next_lcd_state : t_lcd_state;
	
	signal lcd_reset, next_lcd_reset : std_logic;
	signal rom_addr, next_rom_addr : std_logic_vector(num_lcd_addrbits-1 downto 0);
	
	signal i2c_enable, next_i2c_enable : std_logic;
	signal i2c_addr, next_i2c_addr : std_logic_vector(num_i2c_addrbits-1 downto 0);
	signal i2c_data, next_i2c_data : std_logic_vector(num_i2c_databits-1 downto 0);
	signal i2c_last_busy, i2c_cycle : std_logic;

	-- delays
	constant const_wait_prereset : natural := main_clk/1000*10;	-- 10 ms
	constant const_wait_reset : natural := main_clk/1000*1;	-- 1 ms
	constant const_wait_resetted : natural := main_clk/1000*1;	-- 1 ms
	constant const_wait_readrom : natural := main_clk/1000_000*250;	-- 250 us

	-- busy wait counters
	signal wait_counter_prereset, wait_counter_prereset_next : natural range 0 to const_wait_prereset := 0;
	signal wait_counter_prereset_finished, wait_counter_prereset_finished_next : std_logic := '0';

	signal wait_counter_reset, wait_counter_reset_next : natural range 0 to const_wait_reset := 0;
	signal wait_counter_reset_finished, wait_counter_reset_finished_next : std_logic := '0';

	signal wait_counter_resetted, wait_counter_resetted_next : natural range 0 to const_wait_resetted := 0;
	signal wait_counter_resetted_finished, wait_counter_resetted_finished_next : std_logic := '0';

	signal wait_counter_readrom, wait_counter_readrom_next : natural range 0 to const_wait_readrom := 0;
	signal wait_counter_readrom_finished, wait_counter_readrom_finished_next : std_logic := '0';

	-- size constants
	constant init_size : natural := 4;
	constant static_lcd_size : natural := 4*20; --lcd_size;

	-- lcd init command sequence
	type t_init_arr is array(0 to init_size-1) of std_logic_vector(num_lcd_databits-1 downto 0);
	constant init_arr : t_init_arr := (
		-- TODO
		"10000000", "00000001", -- clear screen
		"10000000", "00001111"  -- turn LCD on
	);

	-- cycle counters
	signal init_cycle, next_init_cycle : natural range 0 to init_size := 0;
	signal write_cycle, next_write_cycle : integer range -1 to lcd_size := -1;

	-- control byte: only data bytes following
	constant ctrl_onlydata : std_logic_vector(num_i2c_databits-1 downto 0) := "01000000";

begin

	-- outputs
	out_lcd_reset <= lcd_reset;
	out_i2c_enable <= i2c_enable;
	out_i2c_addr <= i2c_addr;
	out_i2c_data <= i2c_data;
	out_rom_addr <= rom_addr;


	-- rising edge of i2c busy signal
	i2c_cycle <= (in_i2c_busy) and (not i2c_last_busy);


	proc_states : process(in_clk, in_reset) begin
		-- reset
		if in_reset = '0' then
			lcd_state <= Wait_Reset;
			lcd_reset <= '1';

			rom_addr <= (others=>'0');

			i2c_enable <= '0';
			i2c_addr <= (others=>'0');
			i2c_data <= (others=>'0');
			i2c_last_busy <= '0';

			init_cycle <= 0;
			write_cycle <= -1;

			wait_counter_prereset <= 0;
			wait_counter_reset <= 0;
			wait_counter_resetted <= 0;
			wait_counter_readrom <= 0;

			wait_counter_prereset_finished <= '0';
			wait_counter_reset_finished <= '0';
			wait_counter_resetted_finished <= '0';
			wait_counter_readrom_finished <= '0';

		-- clock
		elsif rising_edge(in_clk) then
			lcd_state <= next_lcd_state;
			lcd_reset <= next_lcd_reset;

			rom_addr <= next_rom_addr;

			i2c_enable <= next_i2c_enable;
			i2c_addr <= next_i2c_addr;
			i2c_data <= next_i2c_data;
			i2c_last_busy <= in_i2c_busy;

			init_cycle <= next_init_cycle;
			write_cycle <= next_write_cycle;

			wait_counter_prereset <= wait_counter_prereset_next;
			wait_counter_reset <= wait_counter_reset_next;
			wait_counter_resetted <= wait_counter_resetted_next;
			wait_counter_readrom <= wait_counter_readrom_next;

			wait_counter_prereset_finished <= wait_counter_prereset_finished_next;
			wait_counter_reset_finished <= wait_counter_reset_finished_next;
			wait_counter_resetted_finished <= wait_counter_resetted_finished_next;
			wait_counter_readrom_finished <= wait_counter_readrom_finished_next;

		end if;
	end process;


	-- next timer counters
	wait_counter_prereset_next <= wait_counter_prereset + 1
		when lcd_state=Wait_Reset and wait_counter_prereset/=const_wait_prereset
		else 0;
	wait_counter_prereset_finished_next <= '1' when wait_counter_prereset=const_wait_prereset else '0';

	wait_counter_reset_next <= wait_counter_reset + 1
		when lcd_state=Reset and wait_counter_reset/=const_wait_reset
		else 0;
	wait_counter_reset_finished_next <= '1' when wait_counter_reset=const_wait_reset else '0';

	wait_counter_resetted_next <= wait_counter_resetted + 1
		when lcd_state=Resetted and wait_counter_resetted/=const_wait_resetted
		else 0;
	wait_counter_resetted_finished_next <= '1' when wait_counter_resetted=const_wait_resetted else '0';

	wait_counter_readrom_next <= wait_counter_readrom + 1
		when lcd_state=Wait_ReadROM and wait_counter_readrom/=const_wait_readrom
		else 0;
	wait_counter_readrom_finished_next <= '1' when wait_counter_readrom=const_wait_readrom else '0';


	proc_transitions : process(
		lcd_state, i2c_cycle, init_cycle, write_cycle, 
		lcd_reset, rom_addr, in_rom_word,
		i2c_enable, i2c_addr, i2c_data, in_i2c_busy,
		wait_counter_prereset_finished, wait_counter_reset_finished, wait_counter_resetted_finished, wait_counter_readrom_finished)
	begin
		-- defaults
		next_lcd_state <= lcd_state;
		next_lcd_reset <= lcd_reset;

		next_rom_addr <= rom_addr;

		next_i2c_enable <= i2c_enable;
		next_i2c_addr <= i2c_addr;
		next_i2c_data <= i2c_data;
	
		next_init_cycle <= init_cycle;
		next_write_cycle <= write_cycle;


		-- fsm
		case lcd_state is

			when Wait_Reset =>	
				if wait_counter_prereset_finished= '1' then
					next_lcd_state <= Reset;
				end if;


			when Reset =>
				next_lcd_reset <= '0';
				
				-- wait
				if wait_counter_reset_finished= '1' then
					next_lcd_state <= Resetted;
				end if;


			when Resetted => 
				next_lcd_reset <= '1';

				-- wait
				if wait_counter_resetted_finished= '1' then
					next_lcd_state <= ReadInitSeq;
				end if;


			when ReadInitSeq =>
				if i2c_cycle='1' then
					next_init_cycle <= init_cycle + 1;
				end if;

				case init_cycle is
					-- sequence finished
					when init_size =>
						next_i2c_enable <= '0';
						if in_i2c_busy='0' then
							next_lcd_state <= Wait_ReadROM;
							next_init_cycle <= 0;
						end if;

					-- read from init ROM
					when others =>
						next_i2c_addr <= i2c_writeaddr;
						next_i2c_data <= init_arr(init_cycle);
						next_i2c_enable <= '1';
				end case;


			when Wait_ReadROM =>
				if wait_counter_readrom_finished= '1' then
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
