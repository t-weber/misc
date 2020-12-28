--
-- seven segment leds
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date oct-2020
-- @license see 'LICENSE.EUPL' file
--


/*
library ieee;
use ieee.std_logic_1164.all;


--
-- package declaration
--
package leds is
	component sevenseg
		generic(
			zero_is_on : in std_logic := '0';
			inverse_numbering : in std_logic := '0'
		);

		port
		(
			in_digit : in std_logic_vector(3 downto 0);
			out_leds : out std_logic_vector(6 downto 0)
		);
	end component;
end package;
*/


-------------------------------------------------------------------------------



library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


--
-- pin declaration
--
entity sevenseg is
	generic(
		zero_is_on : in std_logic := '0';
		inverse_numbering : in std_logic := '0'
	);

	port
	(
		in_digit : in std_logic_vector(3 downto 0);
		out_leds : out std_logic_vector(6 downto 0)
	);
end entity;



--
-- implementation
--
architecture sevenseg_impl of sevenseg is

	type t_ledvec is array(0 to 15) of std_logic_vector(7 downto 0);

	-- constants, see: https://en.wikipedia.org/wiki/Seven-segment_display
	constant ledvec : t_ledvec := 
	(
		x"7e", x"30", x"6d", x"79",	-- 0-3
		x"33", x"5b", x"5f", x"70",	-- 4-7
		x"7f", x"7b", x"77", x"1f",	-- 8-b
		x"4e", x"3d", x"4f", x"47"	-- c-f
	);
	constant ledvec_inv : t_ledvec := 
	(
		x"3f", x"06", x"5b", x"4f",	-- 0-3
		x"66", x"6d", x"7d", x"07",	-- 4-7
		x"7f", x"6f", x"77", x"7c",	-- 8-b
		x"39", x"5e", x"79", x"71"	-- c-f
	);

	signal leds : std_logic_vector(6 downto 0);
begin
	gen_leds : if inverse_numbering='0' generate
		--leds <= 
		--	ledvec(0)(6 downto 0) when in_digit=x"0" else
		--	ledvec(1)(6 downto 0) when in_digit=x"1" else
		--	-- ...
		--	"0000000";
	
		with in_digit select leds <=
			ledvec( 0)(6 downto 0) when x"0",
			ledvec( 1)(6 downto 0) when x"1",
			ledvec( 2)(6 downto 0) when x"2",
			ledvec( 3)(6 downto 0) when x"3",
			ledvec( 4)(6 downto 0) when x"4",
			ledvec( 5)(6 downto 0) when x"5",
			ledvec( 6)(6 downto 0) when x"6",
			ledvec( 7)(6 downto 0) when x"7",
			ledvec( 8)(6 downto 0) when x"8",
			ledvec( 9)(6 downto 0) when x"9",
			ledvec(10)(6 downto 0) when x"a",
			ledvec(11)(6 downto 0) when x"b",
			ledvec(12)(6 downto 0) when x"c",
			ledvec(13)(6 downto 0) when x"d",
			ledvec(14)(6 downto 0) when x"e",
			ledvec(15)(6 downto 0) when x"f",
			"0000000" when others;
	end generate;

	gen_leds_inv : if inverse_numbering='1' generate
		with in_digit select leds <=
			ledvec_inv( 0)(6 downto 0) when x"0",
			ledvec_inv( 1)(6 downto 0) when x"1",
			ledvec_inv( 2)(6 downto 0) when x"2",
			ledvec_inv( 3)(6 downto 0) when x"3",
			ledvec_inv( 4)(6 downto 0) when x"4",
			ledvec_inv( 5)(6 downto 0) when x"5",
			ledvec_inv( 6)(6 downto 0) when x"6",
			ledvec_inv( 7)(6 downto 0) when x"7",
			ledvec_inv( 8)(6 downto 0) when x"8",
			ledvec_inv( 9)(6 downto 0) when x"9",
			ledvec_inv(10)(6 downto 0) when x"a",
			ledvec_inv(11)(6 downto 0) when x"b",
			ledvec_inv(12)(6 downto 0) when x"c",
			ledvec_inv(13)(6 downto 0) when x"d",
			ledvec_inv(14)(6 downto 0) when x"e",
			ledvec_inv(15)(6 downto 0) when x"f",
			"0000000" when others;
	end generate;


	--with zero_is_on select out_leds <=
	--	not leds(6 downto 0) when '1',
	--	leds(6 downto 0) when others;

	gen_leds_zero_on : if zero_is_on='1' generate
		out_leds <= not leds;
	end generate;

	gen_leds_zero_off : if zero_is_on='0' generate
		out_leds <= leds;
	end generate;
end architecture;

