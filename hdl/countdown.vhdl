--
-- vhdl test
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2018
-- @license see 'LICENSE.EUPL' file
--


library ieee;
use ieee.std_logic_1164.all;


--
-- package declaration
--
package tst1 is
	component countdown
		generic
		(
			ctr_bit_len : natural := 16;
			start_timer : std_logic_vector(ctr_bit_len-1 downto 0) := x"ffff"
		);
		port
		(
			clk, reset : in std_logic;
			finished : out std_logic
		);
	end component;
end package;




-------------------------------------------------------------------------------



library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.leds.sevenseg;


--
-- pin declaration
--
entity countdown is
	generic
	(
		ctr_bit_len : natural := 16;
		start_timer : std_logic_vector(ctr_bit_len-1 downto 0) := x"ffff"
	);

	port
	(
		clk : in std_logic := '0';
		reset : in std_logic := '0';
		finished : out std_logic := '0'
	);
end entity;



--
-- actual implementation
--
architecture countdown_impl of countdown is

	signal curtime : std_logic_vector(ctr_bit_len-1 downto 0) := start_timer;

	-- output seven segment led displays
	type t_ledseg is array(0 to ctr_bit_len/4-1) of std_logic_vector(6 downto 0);
	signal ledsegs : t_ledseg;

begin

	genleds : for ledsegidx in 0 to ctr_bit_len/4-1 generate
		ledseg : sevenseg 
			generic map(
				zero_is_on => '0',
				inverse_numbering => '0'
			)
			port map(
				in_digit => curtime(ledsegidx*4+3 downto ledsegidx*4),
				out_leds => ledsegs(ledsegidx)
			);
	end generate;

	clk_proc : process(clk) begin
		if rising_edge(clk) then
			if(reset = '1') then
				curtime <= start_timer;
			elsif(curtime > x"0000") then
				curtime <= std_logic_vector(unsigned(curtime(ctr_bit_len-1 downto 0)) - 1);
			end if;

			finished <= '1' when curtime = x"0000" else '0';
		end if;
	end process;
end architecture;

