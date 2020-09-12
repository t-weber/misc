--
-- vhdl test
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2018
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 tst1.vhdl  &&  ghdl -a --std=08 tst1_run.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=tst1.vcd
-- gtkwave tst1.vcd
--

library ieee;
use ieee.std_logic_1164.all;
use work.tst1.all;


entity testbed is
	generic
	(
		thedelay : time := 100 ns
	);
end entity;


architecture thetester of testbed is

	signal theclk : std_logic := '0';
	signal theclk2 : std_logic := '0';

	signal reset : std_logic := '0';

begin
	theclk <= not theclk after thedelay;
	theclk2 <= not theclk2 after thedelay*2;

	timer_comp : countdown port map(clk => theclk, reset => reset);
	timer_comp2 : countdown port map(clk => theclk2, reset => reset);
end architecture;
