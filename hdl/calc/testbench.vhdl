--
-- simple cpu testbench
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date 5-apr-2025
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.conv.all;


entity cpu_tb is
end entity;


architecture cpu_tb_arch of cpu_tb is
	-- main clock
	constant CLKDELAY : time := 2.5 ns;

	-- bus size for addressing individual words
	constant ram_num_addrbits : natural := 8;
	-- word size
	constant ram_num_wordbits : natural := 8;
	-- number of stored words
	constant ram_num_words : natural := 2**ram_num_addrbits;

	signal ram_write0 : std_logic := '0';
	signal ram_addr0, ram_addr1 : std_logic_vector(ram_num_addrbits-1 downto 0)
		:= (others => '0');
	signal ram_write_word0 : std_logic_vector(ram_num_wordbits-1 downto 0)
		:= (others => '0');
	signal ram_read_word0, ram_read_word1 : std_logic_vector(ram_num_wordbits-1 downto 0)
		:= (others => '0');

	signal reg_ip, reg_sp, reg_cycle : std_logic_vector(ram_num_wordbits-1 downto 0)
		:= (others => '0');
	signal reg_instr, reg_op1, reg_op2 : std_logic_vector(ram_num_wordbits-1 downto 0)
		:= (others => '0');
	signal clk, rst : std_logic := '0';

begin
	clk <= not clk after CLKDELAY;
	rst <= '0';

	 -- address where result of function 1 is written to
	ram_addr1 <= x"07";   -- function 1 result address
	--ram_addr1 <= x"60"; -- function 2 counter address


	--==============================================================================
	-- memory
	--==============================================================================
	mem : entity work.ram
		generic map(
			is_rom => '0', num_ports => 2,
			num_addrbits => ram_num_addrbits, num_wordbits => ram_num_wordbits,
			num_words => ram_num_words
		)
		port map(
			in_clk => clk,
			in_write(0) => ram_write0, in_write(1) => '0',
			in_addr(0) => ram_addr0, in_addr(1) => ram_addr1,
			in_word(0) => ram_write_word0, in_word(1) => (others => '0'),
			out_word(0) => ram_read_word0, out_word(1) => ram_read_word1
		);
	--==============================================================================


	--==============================================================================
	-- CPU
	--==============================================================================
	cpu : entity work.cpu
		generic map(
			num_addrbits => ram_num_addrbits, num_wordbits => ram_num_wordbits
		)
		port map(
			in_clk => clk, in_rst => rst,
			out_ram_write => ram_write0, out_ram_addr => ram_addr0,
			in_ram => ram_read_word0, out_ram => ram_write_word0,
			out_ip => reg_ip, out_sp => reg_sp, out_cycle => reg_cycle,
			out_instr => reg_instr, out_op1 => reg_op1, out_op2 => reg_op2
		);
	--==============================================================================

	---------------------------------------------------------------------------
	-- debug output
	---------------------------------------------------------------------------
	report_proc : process(clk)
	begin
		if rising_edge(clk) then
			report  lf &
				"clk = " & std_logic'image(clk) &
				--", mode = " & t_mode'image(cpu_mode) &
				", cycle = " & to_hstring(reg_cycle) &
				", ip = " & to_hstring(reg_ip) &
				", sp = " & to_hstring(reg_sp) &
				", instr = " & to_hstring(reg_instr) &
				", op1 = " & to_hstring(reg_op1) &
				", op2 = " & to_hstring(reg_op2) &
				", " & lf & "ram_addr = " & to_hstring(ram_addr0) &
				", ram_rd = " & to_hstring(ram_read_word0) &
				", ram_wr = " & to_hstring(ram_write_word0) &
				", ram_we = " & std_logic'image(ram_write0) &
				-- function 1 result: should be (8-3+4)*3 = 27 = 0x1b
				", ram[7] = " & to_hstring(ram_read_word1) &
				lf;
		end if;
	end process;
	---------------------------------------------------------------------------

end architecture;
