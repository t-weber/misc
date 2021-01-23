--
-- lfsr test (sim)
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 lfsr.vhdl  &&  ghdl -a --std=08 lfsr_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=lfsr_test.vcd
-- gtkwave lfsr_test.vcd
--

library ieee;
use ieee.std_logic_1164.all;


entity testbed is
	generic(
		constant num_bits : natural := 8;

		constant thedelay : time := 100 ns
	);
end entity;


architecture thetester of testbed is
	signal theclk : std_logic := '0';
	signal val : std_logic_vector(num_bits-1 downto 0);

begin
	theclk <= not theclk after thedelay;

	lfsrmod : entity work.lfsr
		generic map(
			num_bits => num_bits
		)
		port map(
			in_rst=>'0', in_clk=>theclk,
			in_seed=>"00000101", in_setseed=>'0',
			out_val=>val, in_nextval=>'1'
		);
end architecture;
