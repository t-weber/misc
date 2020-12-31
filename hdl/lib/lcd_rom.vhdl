--
-- ROM to initialise LC display
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--
-- reference:
--	- listing 5.24 on pp. 119-120 of the book by Pong P. Chu, 2011, ISBN 978-1-118-00888-1.
--

library ieee;
use ieee.std_logic_1164.all;
use work.conv.all;


entity lcd_rom is
	generic(
		-- bus size for addressing individual words
		constant num_addrbits : natural := 2;
		-- word size
		constant num_wordbits : natural := 8;
		-- number of stored words
		constant num_words : natural := 2**num_addrbits
	);
	
	port(
		in_addr : in std_logic_vector(num_addrbits-1 downto 0);
		out_word : out std_logic_vector(num_wordbits-1 downto 0)
	);
end entity;


architecture lcd_rom_impl of lcd_rom is
	-- a word type
	subtype t_word is std_logic_vector(num_wordbits-1 downto 0);
	-- an array of words type
	type t_words is array(0 to num_words-1) of t_word;

	constant romdata : t_words := (
		-- TODO
		"10000000", "00000001",	-- clear screen
		"10000000", "00001111"	-- turn LCD on
	);

begin
	out_word <= romdata(to_int(in_addr));
end architecture;
