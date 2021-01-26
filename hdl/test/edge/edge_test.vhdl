--
-- edge test (sim)
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 edge.vhdl  &&  ghdl -a --std=08 edge_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=edge_test.vcd
-- gtkwave edge_test.vcd
--

library ieee;
use ieee.std_logic_1164.all;


entity testbed is
	generic(
		constant num_steps : natural := 4;

		constant thedelay : time := 5 ns;
		constant sigdelay : time := 500 ns
	);
end entity;


architecture thetester of testbed is
	signal theclk : std_logic := '0';
	signal thesig : std_logic := '0';
	signal posedge, negedge : std_logic := 'Z';

begin
	theclk <= not theclk after thedelay;
	thesig <= not thesig after sigdelay;

	edgemod_pos : entity work.edge
		generic map(
			num_steps => num_steps,
			pos_edge => '1'
		)
		port map(
			in_clk => theclk,
			in_signal => thesig,
			out_edge => posedge
		);

	edgemod_neg : entity work.edge
		generic map(
			num_steps => num_steps,
			pos_edge => '0'
		)
		port map(
			in_clk => theclk,
			in_signal => thesig,
			out_edge => negedge
		);
end architecture;
