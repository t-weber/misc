--
-- seven segment leds
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date oct-2020
-- @license see 'LICENSE.EUPL' file
--


library ieee;
use ieee.std_logic_1164.all;


--
-- package declaration
--
package leds is
	component sevenseg
		port
		(
			digit : in std_logic_vector(3 downto 0);
			zero_is_on : in std_logic := '0';
			inverse_numbering : in std_logic := '0';

			leds : out std_logic_vector(6 downto 0)
		);

	end component;
end package;



-------------------------------------------------------------------------------



library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


--
-- pin declaration
--
entity sevenseg is
	port
	(
		digit : in std_logic_vector(3 downto 0);
		zero_is_on : in std_logic := '0';
		inverse_numbering : in std_logic := '0';

		leds : out std_logic_vector(6 downto 0)
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


begin
	process(digit, zero_is_on, inverse_numbering)
		variable out_leds : std_logic_vector(6 downto 0);
	begin
		if inverse_numbering = '0' then
			out_leds(6 downto 0) := ledvec(to_integer(unsigned(digit(3 downto 0))))(6 downto 0);
		else
			out_leds(6 downto 0) := ledvec_inv(to_integer(unsigned(digit(3 downto 0))))(6 downto 0);
		end if;

		if zero_is_on = '1' then
			leds <= not out_leds;
		else
			leds <= out_leds;
		end if;
	end process;
end architecture;

