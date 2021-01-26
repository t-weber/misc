--
-- process synchronisation test
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;



entity procs is
	generic
	(
		wait_cycles_A : natural := 1;
		wait_cycles_B : natural := 2
	);

	port
	(
		-- clock and reset
		in_clk, in_rst : in std_logic;

		out_A, out_B : out std_logic
	);
end entity;



architecture procs_impl of procs is
	type t_procstate is (Init, Waiting, Running);
	signal state_A, next_state_A : t_procstate;
	signal state_B, next_state_B : t_procstate;

	signal proc_A_start, next_proc_A_start : std_logic;
	signal proc_B_start, next_proc_B_start : std_logic;

	signal wait_cycle_A, next_wait_cycle_A : natural range 0 to wait_cycles_A;
	signal wait_cycle_B, next_wait_cycle_B : natural range 0 to wait_cycles_B;
begin

	proc_clk : process(in_clk, in_rst)
	begin
		-- reset
		if in_rst='1' then
			state_A <= Init;
			state_B <= Init;
			proc_A_start <= '0';
			proc_B_start <= '0';
			wait_cycle_A <= 0;
			wait_cycle_B <= 0;

		-- clock
		elsif rising_edge(in_clk) then
			state_A <= next_state_A;
			state_B <= next_state_B;
			proc_A_start <= next_proc_A_start;
			proc_B_start <= next_proc_B_start;
			wait_cycle_A <= next_wait_cycle_A;
			wait_cycle_B <= next_wait_cycle_B;
		end if;
	end process;


	proc_A : process(state_A, state_B, proc_A_start, proc_B_start, wait_cycle_A)
	begin
		next_state_A <= state_A;
		next_wait_cycle_A <= wait_cycle_A;
		next_proc_B_start <= proc_B_start;

		case state_A is
			when Init =>
				next_proc_B_start <= '1';

				if wait_cycle_A=wait_cycles_A then
					next_wait_cycle_A <= 0;
					next_state_A <= Waiting;
				else
					next_wait_cycle_A <= wait_cycle_A + 1;
				end if;

			when Waiting =>
				out_A <= '0';
				
				if state_B/=Running and proc_A_start='1' then
					next_proc_B_start <= '0';
					next_state_A <= Running;
				else
					next_proc_B_start <= '1';
				end if;

			when Running =>
				out_A <= '1';
				next_state_A <= Waiting;
		end case;
	end process;


	proc_B : process(state_B, state_A, proc_B_start, proc_A_start, wait_cycle_B)
	begin
		next_state_B <= state_B;
		next_wait_cycle_B <= wait_cycle_B;
		next_proc_A_start <= proc_A_start;

		case state_B is
			when Init =>
				next_proc_A_start <= '1';

				if wait_cycle_B=wait_cycles_B then
					next_wait_cycle_B <= 0;
					next_state_B <= Waiting;
				else
					next_wait_cycle_B <= wait_cycle_B + 1;
				end if;

			when Waiting =>
				out_B <= '0';

				if state_A/=Running and proc_B_start='1' then
					next_proc_A_start <= '0';
					next_state_B <= Running;
				else
					next_proc_A_start <= '1';
				end if;

/*				if state_A=Waiting or proc_B_start='0' then
					next_proc_A_start <= '1';
				else
					next_proc_A_start <= '0';
					next_state_B <= Running;
				end if;
*/

			when Running =>
				out_B <= '1';
				next_state_B <= Waiting;
		end case;
	end process;


end architecture;
