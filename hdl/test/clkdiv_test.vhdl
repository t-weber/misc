--
-- clock divider test
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 conv.vhdl  &&  ghdl -a --std=08 clkdiv.vhdl  &&  ghdl -a --std=08 clkdiv_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=clkdiv_test.vcd
-- gtkwave clkdiv_test.vcd
--

library ieee;
use ieee.std_logic_1164.all;


entity testbed is
	generic
	(
		constant thedelay : time := 100 ns
	);
end entity;


architecture thetester of testbed is

	signal theclk, theclk2 : std_logic := '0';

begin
	theclk <= not theclk after thedelay;

	theclk2_mod : entity work.clkdiv(clkskip_impl)
		generic map(num_ctrbits => 8, shift_bits => 3)
		port map(in_rst => '0', in_clk => theclk, out_clk => theclk2);
end architecture;

