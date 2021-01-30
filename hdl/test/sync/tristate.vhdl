--
-- tri-state signal test (signal with multiple drivers)
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;



entity tristate is
	generic
	(
		active_level : std_logic := '1'
	);

	port
	(
		-- data clock
		in_dataclk : in std_logic;

		-- input data
		in_data : in std_logic;

		-- tristate
		inout_tri : inout std_logic
	);
end entity;



architecture tristate_impl of tristate is
begin

	-- put in_data on the bus wire if the data clock is at active_level,
	-- otherwise leave the bus on high impedance
	inout_tri <= in_data when in_dataclk=active_level else 'Z';

end architecture;
