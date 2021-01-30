--
-- tri-state buffer
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;


entity tristatebuf is
	generic
	(
		active_level : std_logic := '1';
		num_bits : natural := 8
	);

	port
	(
		in_enable : in std_logic;

		-- input data
		in_data : in std_logic_vector(num_bits-1 downto 0);

		-- tristate
		inout_tri : inout std_logic_vector(num_bits-1 downto 0)
	);
end entity;


architecture tristatebuf_impl of tristatebuf is
begin

	-- put in_data on the bus wire if the data clock is at active_level,
	-- otherwise leave the bus on high impedance
	inout_tri <= in_data when in_enable=active_level else (others=>'Z');

end architecture;
