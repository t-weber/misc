--
-- calulator
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;


entity calc is
	generic(
		-- bus size for addressing individual words
		constant ram_num_addrbits : natural := 8;
		-- word size
		constant ram_num_wordbits : natural := 8;
		-- number of stored words
		constant ram_num_words : natural := 2**ram_num_addrbits
	);
	
	port(
		clk : in std_logic;

		key : in std_logic_vector(3 downto 0);
		sw : in std_logic_vector(7 downto 0);

		ledg : out std_logic_vector(7 downto 0);
		ledr : out std_logic_vector(7 downto 0);

		hex0, hex1, hex2, hex3 : out std_logic_vector(6 downto 0)
	);
end entity;


architecture calc_impl of calc is
	type t_disp is array (0 to 4) of std_logic_vector(3 downto 0);
	signal disp : t_disp;
	
	signal ram_write0, ram_write1 : std_logic;
	signal ram_ready0, ram_ready1 : std_logic;
	signal ram_addr0, ram_addr1 : std_logic_vector(ram_num_addrbits-1 downto 0);
	signal ram_write_word0, ram_write_word1  : std_logic_vector(ram_num_wordbits-1 downto 0);
	signal ram_read_word0, ram_read_word1 : std_logic_vector(ram_num_wordbits-1 downto 0);

	signal reg_ip, reg_sp, reg_cycle : std_logic_vector(ram_num_wordbits-1 downto 0);
	signal clkdiv : std_logic;
begin
	--==============================================================================
	-- memory
	--==============================================================================
	mem : entity work.ram
		generic map(
			is_rom=>'0', num_ports=>2,
			num_addrbits=>ram_num_addrbits, num_wordbits=>ram_num_wordbits, num_words=>ram_num_words
		)
		port map(
			in_clk=>clkdiv,
			
			in_write(0)=>ram_write0, in_write(1)=>ram_write1,
			in_addr(0)=>ram_addr0, in_addr(1)=>ram_addr1,
			in_word(0)=>ram_write_word0, in_word(1)=>ram_write_word1, 
			out_word(0)=>ram_read_word0, out_word(1)=>ram_read_word1,
			out_ready(0)=>ram_ready0, out_ready(1)=>ram_ready1
		);
	--==============================================================================

	
	--==============================================================================
	-- CPU
	--==============================================================================
	cpu : entity work.cpu
		generic map(
			num_addrbits=>ram_num_addrbits, num_wordbits=>ram_num_wordbits
		)
		port map(
			in_clk=>clkdiv, in_rst=>key(0), in_ram_ready=>ram_ready0,
			out_ram_write=>ram_write0, out_ram_addr=>ram_addr0,
			in_ram=>ram_read_word0, out_ram=>ram_write_word0,
			out_ip=>reg_ip, out_sp=>reg_sp, out_cycle=>reg_cycle
		);
	--==============================================================================

	
	--==============================================================================
	-- clock divider
	--==============================================================================
	slowclk : entity work.clkdiv
		generic map(
			shift_bits=>24, num_ctrbits=>32
		)
		port map(
			in_clk=>clk, out_clk=>clkdiv
		);
	--==============================================================================


	--==============================================================================
	-- output
	--==============================================================================
	seg0 : entity work.sevenseg generic map(inverse_numbering=>'1', zero_is_on=>'1') port map(in_digit=>disp(0), out_leds=>hex0);	
	seg1 : entity work.sevenseg generic map(inverse_numbering=>'1', zero_is_on=>'1') port map(in_digit=>disp(1), out_leds=>hex1);	
	seg2 : entity work.sevenseg generic map(inverse_numbering=>'1', zero_is_on=>'1') port map(in_digit=>disp(2), out_leds=>hex2);
	seg3 : entity work.sevenseg generic map(inverse_numbering=>'1', zero_is_on=>'1') port map(in_digit=>disp(3), out_leds=>hex3);

	dbgout1 : entity work.dbgout
		generic map(
			num_addrbits=>ram_num_addrbits, num_wordbits=>ram_num_wordbits
		)
		port map(
			in_clk=>clkdiv, in_ram_ready=>ram_ready1,
			out_ram_write=>ram_write1, out_ram_addr=>ram_addr1,
			in_ram=>ram_read_word1,
			in_ip=>reg_ip, in_sp=>reg_sp, in_cycle=>reg_cycle,
			in_show_sp=>sw(0), in_show_instr=>sw(1),
			out_disp0=>disp(0), out_disp1=>disp(1), out_disp2=>disp(2), out_disp3=>disp(3),
			out_disp4=>ledr(3 downto 0), out_disp5=>ledr(7 downto 4)
		);

	ledg(0) <= clkdiv;

	--==============================================================================
	
end architecture;
