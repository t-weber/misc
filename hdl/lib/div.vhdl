--
-- multiply and shift a signal
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date feb-2021
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.conv.all;


--
-- replaces division by multiplication and shift
-- see: https://surf-vhdl.com/how-to-divide-an-integer-by-constant-in-vhdl
--
-- val = num / div
-- val = num*(2^shift_bits/div) * 2^(-shift_bits)
-- val = (num*(2^shift_bits/div)) >> shift_bits
--
-- use the values:
-- mult_const = 2^shift_bits/div
-- mult_bits = ceil(log2(mult_const))
-- shift_bits >= ceil(log2(div))
--
entity div is
	generic(
		constant IN_BITS : natural := 8;
		constant OUT_BITS : natural := 8;
		constant MULT_CONST : natural := 1;
		constant MULT_BITS : natural := 0;
		constant SHIFT_BITS : natural := 0
	);

	port(
		in_num : in std_logic_vector(IN_BITS-1 downto 0);
		out_num : out std_logic_vector(OUT_BITS-1 downto 0)
	);
end entity;


architecture div_impl of div is
	signal num : natural; --range 0 to 2**in_num'length-1;
	signal num_mult : std_logic_vector(IN_BITS+MULT_BITS-1 downto 0);
begin
	num <= to_int(in_num);
	num_mult <= nat_to_logvec(num*MULT_CONST, num_mult'length);
	
	--out_num <= (others=>'0');
	out_num <= num_mult(out_num'length-1+SHIFT_BITS downto SHIFT_BITS);
end architecture;
