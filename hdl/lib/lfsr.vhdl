--
-- lfsr
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
-- @see https://de.wikipedia.org/wiki/Linear_r%C3%BCckgekoppeltes_Schieberegister
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
--use work.conv.all;


entity lfsr is
	generic(
		-- number of bits for shift register
		constant num_bits : natural := 8
	);

	port(
		-- clock and reset
		in_clk, in_rst : in std_logic;

		-- initial value
		in_seed : in std_logic_vector(num_bits-1 downto 0);

		-- enable setting of seed
		in_setseed : in std_logic;

		-- get next value
		in_nextval : in std_logic;

		-- final value
		out_val : out std_logic_vector(num_bits-1 downto 0)
	);
end entity;


architecture lfsr_impl of lfsr is
	signal val, next_val : std_logic_vector(num_bits-1 downto 0) := in_seed;

	function get_next_val(vec : std_logic_vector) return std_logic_vector is
	begin
		-- normal left shift
		-- return vec(num_bits-2 downto 0) & vec(num_bits-1);

		-- normal right shift
		-- return vec(0) & vec(num_bits-1 downto 1);

		-- example, x^8 + x^7 + x^6 + 1
		return (vec(0) xor vec(1) xor vec(2))
			& vec(num_bits-1 downto 1);
	end function;

begin
	-- output
	out_val <= val;

	process(in_clk, in_rst) begin
		-- reset
		if in_rst='1' then
			val <= (others=>'0');

		-- clock
		elsif rising_edge(in_clk) then
			if in_nextval='1' then
				-- next value
				val <= next_val;
			elsif in_setseed='1' then
				-- set new seed
				val <= in_seed;
			end if;
		end if;
	end process;

	next_val <= get_next_val(val);
end architecture;
