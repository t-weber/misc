--
-- simple cpu
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--

/*
 * opcodes
 * =======
 * 0x00: halt
 * 0xff: nop
 *
 * arithmethic operations
 * ----------------------
 * 0x01: add
 * 0x02; sub
 * 0x03: unary minus
 * 0x04: mul
 * 0x05: div
 * 0x06: mod
 *
 * stack operations
 * ----------------
 * 0x10: push <val>
 * 0x11: pop
 * 0x12: push value at address
 *
 * jumps
 * -----
 * 0x20: absolute branch
 * 0x21: conditional, absolute branch
 * 0x22: call
 *
 * logical operations
 * ------------------
 * 0x30: equal
 * 0x31: unequal
 * 0x35: less than
 * 0x36: greater than
 * 0x37: less than or equal
 * 0x38: greater than or equal
 * 0x3a: logical not
 *
 * bitwise operations
 * ------------------
 * 0x40: bitwise or
 * 0x41: bitwise and
 * 0x42: bitwise not
 * 0x45: bit shift left
 * 0x46: bit shift right
 */

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.conv.all;


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
		out_ram_addr : out std_logic_vector(num_addrbits-1 downto 0);
		in_ram : in std_logic_vector(num_wordbits-1 downto 0);
		out_ram : out std_logic_vector(num_wordbits-1 downto 0);

		-- output instruction and stack pointer for debugging
		out_ip, out_sp, out_cycle : out std_logic_vector(num_wordbits-1 downto 0);
		out_instr, out_op1, out_op2 : out std_logic_vector(num_wordbits-1 downto 0)
	);
end entity;



architecture cpu_impl of cpu is
	type t_mode is (Fetch_Instr, Fetched_Instr, Exec_Instr);
	signal cur_mode, next_mode : t_mode := Fetch_Instr;

	-- instruction pointer
	signal reg_ip, next_ip : std_logic_vector(num_wordbits-1 downto 0)
		:= (others => '0');
	-- stack pointer
	signal reg_sp, next_sp : std_logic_vector(num_wordbits-1 downto 0)
		:= (others => '0');
	-- current_instruction
	signal reg_instr, next_instr : std_logic_vector(num_wordbits-1 downto 0)
		:= (others => '0');

	-- operand registers
	signal reg_op1, reg_op2, next_op1, next_op2 : std_logic_vector(num_wordbits-1 downto 0)
		:= (others => '0');

	-- cycle for multi-cycle instructions
	signal cur_cycle, next_cycle : integer range 0 to 15 := 0;

begin
	-- status output for debugging
	out_ip <= reg_ip;
	out_sp <= reg_sp;
	out_cycle <= std_logic_vector(to_unsigned(cur_cycle, num_wordbits));
	out_instr <= reg_instr;
	out_op1 <= reg_op1;
	out_op2 <= reg_op2;


	process(in_clk, in_rst)
	begin
		if rising_edge(in_clk) then
			if in_rst = '1' then
				cur_mode <= Fetch_Instr;
				cur_cycle <= 0;

				reg_ip <= (others => '0');
				reg_sp <= (others => '0');

				reg_instr <= (others => '0');
				reg_op1 <= (others => '0');
				reg_op2 <= (others => '0');
			else
				cur_mode <= next_mode;
				cur_cycle <= next_cycle;

				reg_ip <= next_ip;
				reg_sp <= next_sp;

				reg_instr <= next_instr;
				reg_op1 <= next_op1;
				reg_op2 <= next_op2;
			end if;
		end if;
	end process;


	opproc : process(cur_mode, reg_ip, reg_sp, reg_instr,
		reg_op1, reg_op2, cur_cycle, in_ram)

		-- one-operand expression results
		variable umin, lognot, binnot : std_logic_vector(num_wordbits-1 downto 0);
		variable oneop_result : std_logic_vector(num_wordbits-1 downto 0);

		-- two-operand expression results
		variable sum, diff, quot, remainder : std_logic_vector(num_wordbits-1 downto 0);
		variable prod : std_logic_vector(2*num_wordbits-1 downto 0);
		variable logequ, lognequ : std_logic_vector(num_wordbits-1 downto 0);
		variable logless, loggreater : std_logic_vector(num_wordbits-1 downto 0);
		variable loglessequ, loggreaterequ : std_logic_vector(num_wordbits-1 downto 0);
		variable binor, binand : std_logic_vector(num_wordbits-1 downto 0);
		variable bitshift_l, bitshift_r : std_logic_vector(num_wordbits-1 downto 0) := (others => '0');
		variable twoop_result : std_logic_vector(num_wordbits-1 downto 0);

	begin
		-- default: keep current values
		next_mode <= cur_mode;
		next_cycle <= cur_cycle;

		next_ip <= reg_ip;
		next_sp <= reg_sp;

		next_instr <= reg_instr;
		next_op1 <= reg_op1;
		next_op2 <= reg_op2;

		out_ram_write <= '0';
		out_ram <= (others => '0');
		out_ram_addr <= (others => '0');

		-- one-operand expression results
		umin := std_logic_vector(0 - unsigned(reg_op1));
		lognot := x"01" when reg_op1 = x"00" else (others => '0');
		binnot := not reg_op1;

		oneop_result :=
			umin when reg_instr = x"03" else
			lognot when reg_instr = x"3a" else
			binnot when reg_instr = x"42" else
			(others => '0');

		-- two-operand expression results
		sum := std_logic_vector(unsigned(reg_op2) + unsigned(reg_op1));
		diff := std_logic_vector(unsigned(reg_op2) - unsigned(reg_op1));
		prod := std_logic_vector(unsigned(reg_op2) * unsigned(reg_op1));
		quot := (others => '1') when unsigned(reg_op1) = 0 else
			std_logic_vector(unsigned(reg_op2) / unsigned(reg_op1));
		remainder := (others => '1') when unsigned(reg_op1) = 0 else
			std_logic_vector(unsigned(reg_op2) mod unsigned(reg_op1));
		binor := reg_op2 or reg_op1;
		binand := reg_op2 and reg_op1;
		bitshift_l := (others => '0');
		bitshift_r := (others => '0');
		bitshift_l(num_wordbits - 1 downto to_int(reg_op1(3 downto 0)))
			:= reg_op2(num_wordbits - to_int(reg_op1(3 downto 0)) - 1 downto 0);
		--bitshift_l(to_int(reg_op1(3 downto 0)) - 1 downto 0) := (others => '0'); --when to_int(reg_op1(3 downto 0)) > 0;
		bitshift_r(num_wordbits - to_int(reg_op1(3 downto 0)) - 1 downto 0)
			:= reg_op2(num_wordbits - 1 downto to_int(reg_op1(3 downto 0)));
		--bitshift_r(num_wordbits - 1 downto to_int(reg_op1(3 downto 0))) := (others => '0'); --when to_int(reg_op1(3 downto 0)) < 8;
		logequ := x"01" when reg_op2 = reg_op1 else (others => '0');
		lognequ := x"01" when reg_op2 /= reg_op1 else (others => '0');
		logless := x"01" when unsigned(reg_op2) < unsigned(reg_op1) else (others => '0');
		loggreater := x"01" when unsigned(reg_op2) > unsigned(reg_op1) else (others => '0');
		loglessequ := x"01" when unsigned(reg_op2) <= unsigned(reg_op1) else (others => '0');
		loggreaterequ := x"01" when unsigned(reg_op2) >= unsigned(reg_op1) else (others => '0');

		twoop_result :=
			sum when reg_instr = x"01" else
			diff when reg_instr = x"02" else
			prod(num_wordbits-1 downto 0) when reg_instr = x"04" else
			quot when reg_instr = x"05" else
			remainder when reg_instr = x"06" else
			logequ when reg_instr = x"30" else
			lognequ when reg_instr = x"31" else
			logless when reg_instr = x"35" else
			loggreater when reg_instr = x"36" else
			loglessequ when reg_instr = x"37" else
			loggreaterequ when reg_instr = x"38" else
			binor when reg_instr = x"40" else
			binand when reg_instr = x"41" else
			bitshift_l when reg_instr = x"45" else
			bitshift_r when reg_instr = x"46" else
			(others => '0');


		case cur_mode is
			--
			-- get instruction from ram
			--
			when Fetch_Instr =>
				out_ram_addr <= reg_ip;
				next_mode <= Fetched_Instr;


			when Fetched_Instr =>
				out_ram_addr <= reg_ip;
				next_instr <= in_ram;
				next_cycle <= 0;
				next_mode <= Exec_Instr;


			-- -------------------------------------------------------------------
			-- execute opcode
			-- -------------------------------------------------------------------
			when Exec_Instr =>
				case reg_instr is

					--
					-- halt
					--
					when x"00" =>
						null;


					--
					-- nop
					--
					when x"ff" =>
						-- next instruction
						next_ip <= std_logic_vector(unsigned(reg_ip)+1);
						next_mode <= Fetch_Instr;


					--
					-- one-operand expressions
					--
					when x"03" | x"3a" | x"42" =>
						case cur_cycle is
							-- pop operand
							when 0 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 1;

							-- pop operand
							when 1 =>
								out_ram_addr <= reg_sp;
								next_op1 <= in_ram;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 2;

							-- push result
							when 2 =>
								out_ram <= oneop_result;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 3;

							-- push result
							when 3 =>
								out_ram_write <= '1';
								out_ram <= oneop_result;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);

								next_sp <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 4;

							-- next instruction
							when 4 =>
								next_ip <= std_logic_vector(unsigned(reg_ip)+1);
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;


					--
					-- two-operand expressions
					--
					when x"01" | x"02" | x"04" | x"05" | x"06" |
						x"30" | x"31" | x"35" | x"36" | x"37" | x"38" |
						x"40" | x"41" | x"45" | x"46" =>
						case cur_cycle is
							-- pop operand 1
							when 0 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 1;

							-- pop operand 1
							when 1 =>
								out_ram_addr <= reg_sp;
								next_op1 <= in_ram;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 2;

							-- pop operand 2
							when 2 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 3;

							-- pop operand 2
							when 3 =>
								out_ram_addr <= reg_sp;
								next_op2 <= in_ram;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 4;

							-- push result
							when 4 =>
								out_ram <= twoop_result;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 5;

							-- push result
							when 5 =>
								out_ram_write <= '1';
								out_ram <= twoop_result;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);

								next_sp <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 6;

							-- next instruction
							when 6 =>
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
								next_cycle <= 1;

							-- get data from the word after the instruction
							when 1 =>
								out_ram_addr <= std_logic_vector(unsigned(reg_ip)+1);
								next_op1 <= in_ram; -- save data to op1 register
								next_cycle <= 2;

							-- push data on stack
							when 2 =>
								out_ram <= reg_op1;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 3;

							-- push data on stack
							when 3 =>
								out_ram_write <= '1';
								out_ram <= reg_op1;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);

								next_sp <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 4;

							-- next instruction
							when 4 =>
								next_ip <= std_logic_vector(unsigned(reg_ip)+2);
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;


					--
					-- pop value to address
					--
					when x"11" =>
						case cur_cycle is
							-- pop address
							when 0 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 1;

							-- pop address
							when 1 =>
								out_ram_addr <= reg_sp;
								next_op1 <= in_ram;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 2;

							-- pop value
							when 2 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 3;

							-- pop value
							when 3 =>
								out_ram_addr <= reg_sp;
								next_op2 <= in_ram;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 4;

							-- write data to address
							when 4 =>
								out_ram <= reg_op2;
								out_ram_addr <= reg_op1;
								next_cycle <= 5;

							-- write data to address
							when 5 =>
								out_ram_write <= '1';
								out_ram <= reg_op2;
								out_ram_addr <= reg_op1;
								next_cycle <= 6;

							-- next instruction
							when 6 =>
								next_ip <= std_logic_vector(unsigned(reg_ip)+1);
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;


					--
					-- push value at address
					--
					when x"12" =>
						case cur_cycle is
							-- pop address
							when 0 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 1;

							-- pop address
							when 1 =>
								out_ram_addr <= reg_sp;
								next_op1 <= in_ram;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 2;

							-- get data from the address
							when 2 =>
								out_ram_addr <= reg_op1;
								next_cycle <= 3;

							-- get data from the address
							when 3 =>
								out_ram_addr <= reg_op1;
								next_op2 <= in_ram; -- save data to op2 register
								next_cycle <= 4;

							-- push data on stack
							when 4 =>
								out_ram <= reg_op2;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 5;

							-- push data on stack
							when 5 =>
								out_ram_write <= '1';
								out_ram <= reg_op2;
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);

								next_sp <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 6;

							-- next instruction
							when 6 =>
								next_ip <= std_logic_vector(unsigned(reg_ip)+1);
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;


					--
					-- absolute branching
					--
					when x"20" =>
						case cur_cycle is
							-- pop address
							when 0 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 1;

							-- pop address and set new instruction pointer
							when 1 =>
								out_ram_addr <= reg_sp;
								next_ip <= in_ram;
								next_mode <= Fetch_Instr;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);

							when others =>
								next_cycle <= 0;
						end case;


					--
					-- conditional, absolute branching
					--
					when x"21" =>
						case cur_cycle is
							-- pop address
							when 0 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 1;

							-- pop address
							when 1 =>
								out_ram_addr <= reg_sp;
								next_op1 <= in_ram;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 2;

							-- pop condition value
							when 2 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 3;

							-- pop condition value
							when 3 =>
								out_ram_addr <= reg_sp;
								next_op2 <= in_ram;
								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 4;

							-- set new instruction pointer
							when 4 =>
								if reg_op2=x"00" then
									-- value is 0 -> no branching
									next_ip <= std_logic_vector(unsigned(reg_ip)+1);
								else
									-- value is not 0 -> branch to address
									next_ip <= reg_op1;
								end if;
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;


					--
					-- call
					--
					when x"22" =>
						case cur_cycle is
							-- pop function address
							when 0 =>
								out_ram_addr <= reg_sp;
								next_cycle <= 1;

							-- pop function address
							when 1 =>
								out_ram_addr <= reg_sp;
								next_op1 <= in_ram;
								next_cycle <= 2;

								next_sp <= std_logic_vector(unsigned(reg_sp)+1);
								next_cycle <= 3;

							-- push pointer to next instruction
							when 2 =>
								out_ram <= std_logic_vector(unsigned(reg_ip) + 1);
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 3;

							-- push pointer to next instruction
							when 3 =>
								out_ram_write <= '1';
								out_ram <= std_logic_vector(unsigned(reg_ip) + 1);
								out_ram_addr <= std_logic_vector(unsigned(reg_sp)-1);

								next_sp <= std_logic_vector(unsigned(reg_sp)-1);
								next_cycle <= 4;

							-- branch to function
							when 4 =>
								next_ip <= reg_op1;
								next_mode <= Fetch_Instr;

							when others =>
								next_cycle <= 0;
						end case;
					when others =>
						null;
				end case;
				-- -------------------------------------------------------------------

			when others =>
				null;
		end case;
	end process;
end architecture;
