--
-- divider test
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date feb-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 conv.vhdl  &&  ghdl -a --std=08 div.vhdl  &&  ghdl -a --std=08 div_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=div_test.vcd
-- gtkwave div_test.vcd
--

library ieee;
use ieee.std_logic_1164.all;


entity testbed is
end entity;


--
-- example:
--   div = 1234
--   shift_bits >= ceil(log2(div)), use 16
--   mult_const = 2^shift_bits/div = 53
--   mult_bits = ceil(log2(mult_const)) = 6
--
architecture thetester of testbed is
	constant MULT_CONST : natural := 53;
	constant MULT_BITS : natural := 6;
	constant SHIFT_BITS : natural := 16;

	constant MULT_CONST2 : natural := 106;
	constant MULT_BITS2 : natural := 7;
	constant SHIFT_BITS2 : natural := 17;

	constant MULT_CONST3 : natural := 26;
	constant MULT_BITS3 : natural := 5;
	constant SHIFT_BITS3 : natural := 15;

	constant IN_BITS : natural := 32;
	constant OUT_BITS : natural := 32-SHIFT_BITS;

	signal num : std_logic_vector(IN_BITS-1 downto 0) := x"00ff00ff";	-- 16711935
	signal num_div, num_div2, num_div3 : std_logic_vector(OUT_BITS-1 downto 0);	-- 16711935 / 1234 = 13542

	constant THEDELAY : time := 100 ns;
	signal theclk : std_logic := '0';
begin
	-- no clock needed, just for simulator
	theclk <= not theclk after THEDELAY;

	div_mod : entity work.div
		generic map(
			IN_BITS => IN_BITS, OUT_BITS => OUT_BITS,
			MULT_CONST => MULT_CONST, MULT_BITS => MULT_BITS,
			SHIFT_BITS => SHIFT_BITS
		)
		port map(
			in_num => num, out_num => num_div
		);

	div_mod2 : entity work.div
		generic map(
			IN_BITS => IN_BITS, OUT_BITS => OUT_BITS,
			MULT_CONST => MULT_CONST2, MULT_BITS => MULT_BITS2,
			SHIFT_BITS => SHIFT_BITS2
		)
		port map(
			in_num => num, out_num => num_div2
		);

	div_mod3 : entity work.div
		generic map(
			IN_BITS => IN_BITS, OUT_BITS => OUT_BITS,
			MULT_CONST => MULT_CONST3, MULT_BITS => MULT_BITS3,
			SHIFT_BITS => SHIFT_BITS3
		)
		port map(
			in_num => num, out_num => num_div3
		);
end architecture;
