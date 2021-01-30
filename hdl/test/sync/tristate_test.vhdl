--
-- tristate test (sim)
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 tristate.vhdl  &&  ghdl -a --std=08 tristate_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=tristate_test.vcd
-- gtkwave tristate_test.vcd
--

library ieee;
use ieee.std_logic_1164.all;


entity testbed is
	generic
	(
		constant thedelay : time := 10 ns
	);
end entity;


architecture thetester of testbed is
	signal theclk : std_logic := '0';
	signal sig_tri : std_logic;

begin
	theclk <= not theclk after thedelay;

	proc1 : entity work.tristate
		generic map(
			active_level => '1'
		)
		port map(
			in_dataclk => theclk,
			in_data => '0',
			inout_tri => sig_tri
		);

	proc2 : entity work.tristate
		generic map(
			active_level => '0'
		)
		port map(
			in_dataclk => theclk,
			in_data => '1',
			inout_tri => sig_tri
		);

end architecture;
