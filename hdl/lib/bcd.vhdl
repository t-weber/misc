--
-- bcd conversion
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date feb-2021
-- @license see 'LICENSE.EUPL' file
--
-- @see https://en.wikipedia.org/wiki/Double_dabble
--

library ieee;
use ieee.std_logic_1164.all;
use work.conv.all;


entity bcd is
	generic(
		-- size of the input and output numbers
		constant IN_BITS : natural := 8;
		constant OUT_BITS : natural := 3*4;
		constant NUM_BCD_DIGITS : natural := OUT_BITS/4
	);

	port(
		-- clock and reset
		in_clk, in_rst : in std_logic;

		-- start signal
		in_start : in std_logic;

		-- input
		in_num : in std_logic_vector(IN_BITS-1 downto 0);

		-- output
		out_bcd : out std_logic_vector(OUT_BITS-1 downto 0);

		-- conversion finished (or idle)?
		out_finished : out std_logic
	);
end entity;


architecture bcd_impl of bcd is
	type t_state is (Idle, Shift, Add, NextIndex);
	signal state, state_next : t_state := Shift;

	signal bcdnum, bcdnum_next : std_logic_vector(OUT_BITS-1 downto 0) := (others=>'0');
	signal bitidx, bitidx_next : natural range IN_BITS-1 downto 0 := IN_BITS-1;
	signal bcdidx, bcdidx_next : natural range NUM_BCD_DIGITS-1 downto 0 := NUM_BCD_DIGITS-1;
begin

	-- output
	out_bcd <= bcdnum;
	out_finished <= '1' when state=Idle else '0';


	-- clock process
	clk_proc : process(in_clk, in_rst)
	begin
		if in_rst='1' then
			state <= Shift;
			bcdnum <= (others=>'0');
			bitidx <= IN_BITS-1;
			bcdidx <= NUM_BCD_DIGITS-1;

		elsif rising_edge(in_clk) then
			state <= state_next;
			bcdnum <= bcdnum_next;
			bitidx <= bitidx_next;
			bcdidx <= bcdidx_next;
		end if;
	end process;


	-- conversion process
	conv_proc : process(state, bitidx, bcdidx, bcdnum)
	begin
		-- save registers
		state_next <= state;
		bcdnum_next <= bcdnum;
		bitidx_next <= bitidx;
		bcdidx_next <= bcdidx;


		case state is
			-- wait for start signal
			when Idle =>
				if in_start='1' then
					-- reset
					bcdnum_next <= (others=>'0');
					bitidx_next <= IN_BITS-1;
					bcdidx_next <= NUM_BCD_DIGITS-1;
					state_next <= Shift;
				end if;

			-- shift left
			when Shift =>
				bcdnum_next(OUT_BITS-1 downto 1) <= bcdnum(OUT_BITS-2 downto 0);
				bcdnum_next(0) <= in_num(bitidx);

				-- no addition for last index
				if bitidx /= 0 then
					state_next <= Add;
				else
					state_next <= Idle;
				end if;

			-- add 3 if bcd digit >= 5
			when Add =>
				-- check if the bcd digit is >= 5
				if to_int(bcdnum(bcdidx*4+3 downto bcdidx*4)) >= 5 then
					-- if so, add 3
					bcdnum_next(OUT_BITS-1 downto bcdidx*4) <=
						inc_logvec(bcdnum(OUT_BITS-1 downto bcdidx*4), 3);
				end if;

				if bcdidx /= 0 then
					bcdidx_next <= bcdidx-1;
				else
					bcdidx_next <= NUM_BCD_DIGITS-1;
					state_next <= NextIndex;
				end if;

			-- next bit
			when NextIndex =>
				bitidx_next <= bitidx-1;
				state_next <= Shift;
		end case;
	end process;
end architecture;
