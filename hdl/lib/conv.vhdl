--
-- conversions
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;
--use ieee.std_logic_arith.all;
use ieee.numeric_std.all;


package conv is
	-- array types
	type t_logicarray is array(natural range <>) of std_logic;
	type t_logicvecarray is array(natural range <>) of std_logic_vector;

	-- std_logic_vector -> integer
	function to_int(vec : std_logic_vector) return integer;
end package;


package body conv is
	--
	-- std_logic_vector -> integer
	--
	function to_int(vec : std_logic_vector) return integer is
	begin
		return to_integer(unsigned(vec));
	end function;
end package body;
