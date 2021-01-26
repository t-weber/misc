--
-- edge test (sim)
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 procs.vhdl  &&  ghdl -a --std=08 procs_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=procs_test.vcd
-- gtkwave procs_test.vcd
--

library ieee;
use ieee.std_logic_1164.all;


entity testbed is
	generic
	(
		constant thedelay : time := 5 ns
	);
end entity;


architecture thetester of testbed is
	signal theclk : std_logic := '0';
	signal sig_A, sig_B : std_logic;

begin
	theclk <= not theclk after thedelay;

	procs : entity work.procs
		port map(
			in_clk => theclk,
			in_rst => '0',

			out_A => sig_A,
			out_B => sig_B
		);

end architecture;
