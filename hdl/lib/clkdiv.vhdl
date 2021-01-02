--
-- clock divider
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.conv.all;


entity clkdiv is
	generic(
		-- number of bits for counter
		constant num_ctrbits : natural := 8;
		constant shift_bits : natural := 0
);

	port(
		in_rst : in std_logic;
		in_clk : in std_logic;
		out_clk : out std_logic
	);
end entity;


--
-- skips shift_bits clock cycles
--
architecture clkskip_impl of clkdiv is
	signal ctr, next_ctr : std_logic_vector(num_ctrbits-1 downto 0) := (others=>'0');
	signal ctr_fin : std_logic := '0';
	signal slow_clk, next_slow_clk : std_logic := '0';
begin
	-- output clock
	--out_clk <= slow_clk;
	out_clk <= in_clk when shift_bits=1 else slow_clk;

	ctrprc : process(in_clk) begin
		if in_rst='1' then
			ctr <= (others => '0');
			slow_clk <= '0';
		elsif rising_edge(in_clk) then
			ctr <= next_ctr;
			slow_clk <= next_slow_clk;
		end if;
	end process;

	clkproc : process(ctr_fin) begin
		next_slow_clk <= slow_clk;

		if ctr_fin='1' then
			next_slow_clk <= not slow_clk;
		end if;
	end process;

	-- count cycles to skip
	next_ctr <=
		int_to_logvec(0, num_ctrbits) 
			when ctr = nat_to_logvec(shift_bits-1, num_ctrbits)
		else inc_logvec(ctr, 1);

	ctr_fin <=
		'1' when ctr = nat_to_logvec(shift_bits-1, num_ctrbits)
		else '0';
end architecture;


--
-- standard behaviour:
-- divides clock by 2**shift_bits
--
architecture clkdiv_impl of clkdiv is
	signal ctr, next_ctr : std_logic_vector(num_ctrbits-1 downto 0) := (others=>'0');
begin

	ctrprc : process(in_clk) begin
		if in_rst='1' then
			ctr <= (others => '0');
		elsif rising_edge(in_clk) then
			ctr <= next_ctr;
		end if;
	end process;

	next_ctr <= inc_logvec(ctr, 1);
	out_clk <= ctr(shift_bits);
end architecture;
