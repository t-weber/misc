--
-- simple cpu
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity cpu is
	generic(
		-- bus size for addressing individual words
		constant num_addrbits : natural := 8;
		-- word size
		constant num_wordbits : natural := 8
	);

	port(
		in_clk, in_rst : in std_logic;

		-- ram interface
		out_ram_write : out std_logic;
		in_ram_ready : in std_logic;
		out_ram_addr : out std_logic_vector(num_addrbits-1 downto 0);
		in_ram : in std_logic_vector(num_wordbits-1 downto 0);
		out_ram : out std_logic_vector(num_wordbits-1 downto 0);

		-- output instruction and stack pointer for debugging
		out_ip, out_sp, out_cycle : out std_logic_vector(num_wordbits-1 downto 0)
	);
end entity;



architecture cpu_impl of cpu is
	type t_mode is (Fetch_Instr, Fetched_Instr, Exec_Instr);
	signal cur_mode, next_mode : t_mode;

	-- instruction pointer
	signal reg_ip, next_ip : std_logic_vector(num_wordbits-1 downto 0);
	-- stack pointer
	signal reg_sp, next_sp : std_logic_vector(num_wordbits-1 downto 0);
	-- current_instruction
	signal reg_instr, next_instr : std_logic_vector(num_wordbits-1 downto 0);

	-- operand  registers
	signal reg_op1, reg_op2, next_op1, next_op2 : std_logic_vector(num_wordbits-1 downto 0);
	
	-- cycle for multi-cycle instructions
	signal cur_cycle, next_cycle : integer range 0 to 15 := 0;
	
begin
	-- status output for debugging
	out_ip <= reg_ip;
	out_sp <= reg_sp;
	out_cycle <= std_logic_vector(to_unsigned(cur_cycle, num_wordbits));


	process(in_clk, in_rst)
	begin
		if in_rst='0' then
			cur_mode <= Fetch_Instr;
			reg_ip <= x"00";
			reg_sp <= x"00";
			reg_instr <= x"00";
			cur_cycle <= 0;

		elsif rising_edge(in_clk) then
			cur_mode <= next_mode;

			reg_ip <= next_ip;
			reg_sp <= next_sp;
			
			reg_op1 <= next_op1;
			reg_op2 <= next_op2;
			
			reg_instr <= next_instr;
			cur_cycle <= next_cycle;
		end if;
	end process;

	
	opproc : process(cur_mode, reg_ip, reg_sp, reg_instr, reg_op1, reg_op2, cur_cycle, in_ram, in_ram_ready)
	begin
		-- default: keep current values
		next_mode <= cur_mode;

		next_ip <= reg_ip;
		next_sp <= reg_sp;

		next_op1 <= reg_op1;
		next_op2 <= reg_op2;

		next_instr <= reg_instr;
		next_cycle <= cur_cycle;

		out_ram_write <= '0';
		out_ram <= x"00";
		out_ram_addr <= x"00";


		case cur_mode is
			--
			-- get instruction from ram
			--
			when Fetch_Instr =>
				out_ram_addr <= reg_ip;
				if in_ram_ready='1' then
					next_mode <= Fetched_Instr;
				end if;


			when Fetched_Instr =>
				out_ram_addr <= reg_ip;
				if in_ram_ready='1' then
					next_instr <= in_ram;
					next_cycle <= 0;
					next_mode <= Exec_Instr;
				end if;


			--
			-- execute opcode
			--
			when Exec_Instr =>		
				case reg_instr is

					--
					-- halt
					--
					when x"00" =>
						null;


					--
					-- add
					--
					when x"01" =>
						case cur_cycle is
							-- pop operand 1
							when 0 =>
								out_ram_addr <= reg_sp;
								if in_ram_ready='1' then
									next_cycle <= 1;
								end if;

							-- pop operand 1
							when 1 =>
								out_ram_addr <= reg_sp;

								if in_ram_ready='1' then
									next_op1 <= in_ram;
									next_sp <= std_logic_vector(unsigned(reg_sp)+1);
									next_cycle <= 2;
								end if;

							-- pop operand 2
							when 2 =>
								out_ram_addr <= reg_sp;
								if in_ram_ready='1' then
									next_cycle <= 3;
								end if;

							-- pop operand 2
							when 3 =>
								out_ram_addr <= reg_sp;

								if in_ram_ready='1' then
									next_op2 <= in_ram;
									next_sp <= std_logic_vector(unsigned(reg_sp)+1);
									next_cycle <= 4;
								end if;

							-- push result
							when 4 =>
								out_ram_write <= '1';
								out_ram <= std_logic_vector(unsigned(reg_op1) + unsigned(reg_op2));
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);
								
								next_sp <= std_logic_vector(unsigned(reg_sp)-1);

								if in_ram_ready='1' then
									next_cycle <= 5;
								end if;

							-- next instruction
							when 5 =>
								next_ip <= std_logic_vector(unsigned(reg_ip)+1);
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;


					--
					-- sub
					--
					when x"02" =>
						case cur_cycle is
							-- pop operand 1
							when 0 =>
								out_ram_addr <= reg_sp;
								if in_ram_ready='1' then
									next_cycle <= 1;
								end if;

							-- pop operand 1
							when 1 =>
								out_ram_addr <= reg_sp;

								if in_ram_ready='1' then
									next_op1 <= in_ram;
									next_sp <= std_logic_vector(unsigned(reg_sp)+1);
									next_cycle <= 2;
								end if;

							-- pop operand 2
							when 2 =>
								out_ram_addr <= reg_sp;
								if in_ram_ready='1' then
									next_cycle <= 3;
								end if;

							-- pop operand 2
							when 3 =>
								out_ram_addr <= reg_sp;

								if in_ram_ready='1' then
									next_op2 <= in_ram;
									next_sp <= std_logic_vector(unsigned(reg_sp)+1);
									next_cycle <= 4;
								end if;

							-- push result
							when 4 =>
								out_ram_write <= '1';
								out_ram <= std_logic_vector(unsigned(reg_op1) - unsigned(reg_op2));
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);

								next_sp <= std_logic_vector(unsigned(reg_sp)-1);

								if in_ram_ready='1' then
									next_cycle <= 5;
								end if;

							-- next instruction
							when 5 =>
								next_ip <= std_logic_vector(unsigned(reg_ip)+1);
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;


					--
					-- push value
					--
					when x"10" =>
						case cur_cycle is
							-- get data from the word after the instruction
							when 0 =>
								out_ram_addr <= std_logic_vector(unsigned(reg_ip)+1);
								if in_ram_ready='1' then
									next_cycle <= 1;
								end if;

							-- get data from the word after the instruction
							when 1 =>
								out_ram_addr <= std_logic_vector(unsigned(reg_ip)+1);

								if in_ram_ready='1' then
									next_op1 <= in_ram; -- save data to op1 register
									next_cycle <= 2;
								end if;

							-- push data on stack
							when 2 =>
								out_ram_write <= '1';
								out_ram <= reg_op1;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);

								next_sp <= std_logic_vector(unsigned(reg_sp)-1);

								if in_ram_ready='1' then
									next_cycle <= 3;
								end if;

							-- next instruction
							when 3 =>
								next_ip <= std_logic_vector(unsigned(reg_ip)+2);
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;


					when others =>
						null;
				end case;


			when others =>
				null;
		end case;
	end process;
end architecture;
