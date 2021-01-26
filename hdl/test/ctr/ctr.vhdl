--
-- counter timing comparison test
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;
--use work.conv.all;


entity ctr is
	port(
		-- clock
		in_clk : in std_logic;

		-- reset
		in_reset : in std_logic;

		-- counters
		out_ctr_finished : out std_logic;
		out_ctr2_finished : out std_logic
	);
end entity;



architecture ctr_impl of ctr is
	constant delay : natural := 5;

	signal counter : natural range 0 to delay := 0;
	signal counter_finished : std_logic := '0';

	signal counter2, counter2_next : natural range 0 to delay := 0;
	signal counter2_finished, counter2_finished_next : std_logic := '0';


begin

	-- outputs
	out_ctr_finished <= counter_finished;
	out_ctr2_finished <= counter2_finished;


	-----------------------------------------------------------------------
	-- counter 1
	-----------------------------------------------------------------------
	-- process for timers
	proc_timer : process(in_clk, in_reset) begin
		-- reset
		if in_reset = '1' then
			counter <= 0;
			counter_finished <= '0';

		elsif rising_edge(in_clk) then
			if counter = delay then
				counter <= 0;
				counter_finished <= '1';
			else
				counter <= counter + 1;
				counter_finished <= '0';
			end if;
		end if;
	end process;
	-----------------------------------------------------------------------


	-----------------------------------------------------------------------
	-- counter 2
	-----------------------------------------------------------------------
	proc_timer2 : process(in_clk, in_reset) begin
		-- reset
		if in_reset = '1' then
			counter2 <= 0;
			counter2_finished <= '0';

		-- clock
		elsif rising_edge(in_clk) then
			counter2 <= counter2_next;
			counter2_finished <= counter2_finished_next;
		end if;
	end process;

	-- next counter
	counter2_next <= 0 when counter2=delay else counter2 + 1;
	counter2_finished_next <= '1' when counter2=delay else '0';
	-----------------------------------------------------------------------

end architecture;
