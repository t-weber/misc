--
-- clock divider
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity clkdiv is
	generic(
		-- divide clock by 2**...
		constant shift_bits : natural := 0;

		-- number of bits for counter
		constant num_ctrbits : natural := 8
	);

	port(
		in_clk : in std_logic;
		out_clk : out std_logic
	);
end entity;


architecture clkdiv_impl of clkdiv is
	signal ctr, next_ctr : std_logic_vector(num_ctrbits-1 downto 0);
begin

	ctrprc : process(in_clk) begin
		if rising_edge(in_clk) then
			ctr <= next_ctr;
		end if;
	end process;

	next_ctr <= std_logic_vector(unsigned(ctr) + 1);
	out_clk <= ctr(shift_bits);
end architecture;
