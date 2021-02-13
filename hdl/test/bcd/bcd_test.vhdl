--
-- bcd test sim
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date feb-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 conv.vhdl  &&  ghdl -a --std=08 bcd.vhdl  &&  ghdl -a --std=08 bcd_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=bcd_test.vcd
-- gtkwave bcd_test.vcd
--

library ieee;
use ieee.std_logic_1164.all;


entity testbed is
end entity;


architecture thetester of testbed is
	constant thedelay : time := 100 ns;
	signal theclk : std_logic := '0';

	signal bin_num : std_logic_vector(7 downto 0) := "10010101"; -- 149
	signal bcd_num : std_logic_vector(12 downto 0);
	signal finished : std_logic;

begin
	theclk <= not theclk after thedelay;

	bcd_mod : entity work.bcd
		generic map(IN_BITS=>bin_num'length, OUT_BITS=>bcd_num'length,
			NUM_BCD_DIGITS=>3)
		port map(in_clk=>theclk, in_rst=>'0',
			in_num=>bin_num, out_bcd=>bcd_num,
			out_finished=>finished);
end architecture;
