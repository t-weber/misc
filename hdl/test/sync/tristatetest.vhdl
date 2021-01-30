--
-- using a tri-state buffer like a multiplexer
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;


entity calc is
	port(
		sw : in std_logic_vector(8 downto 0);
		hex0 : out std_logic_vector(6 downto 0)
	);
end entity;


architecture calc_impl of calc is
	signal tribuf : std_logic_vector(3 downto 0);
begin

	seg0 : entity work.sevenseg 
		generic map(inverse_numbering=>'1', zero_is_on=>'1')
		port map(in_digit=>tribuf, out_leds=>hex0);

	buf1 : entity work.tristatebuf
		generic map(active_level=>'0', num_bits=>4)
		port map(in_enable=>sw(8), in_data=>sw(3 downto 0), inout_tri=>tribuf);

	buf2 : entity work.tristatebuf
		generic map(active_level=>'1', num_bits=>4)
		port map(in_enable=>sw(8), in_data=>sw(7 downto 4), inout_tri=>tribuf);

end architecture;
