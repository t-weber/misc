--
-- counter comparison test (sim)
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 ctr.vhdl  &&  ghdl -a --std=08 ctr_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=ctr_test.vcd
-- gtkwave ctr_test.vcd
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
	signal theclk : std_logic := '0';
	signal ctr_fin, ctr2_fin : std_logic := '0';

begin
	theclk <= not theclk after thedelay;

	ctrmod : entity work.ctr
		port map(
			in_reset=>'0', in_clk=>theclk,
			out_ctr_finished=>ctr_fin, out_ctr2_finished=>ctr2_fin
		);
end architecture;
