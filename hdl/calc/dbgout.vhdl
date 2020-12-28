--
-- debug output
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity dbgout is
	generic(
		-- bus size for addressing individual words
		constant num_addrbits : natural := 8;
		-- word size
		constant num_wordbits : natural := 8
	);

	port(
		in_clk : in std_logic;

		-- cpu status
		in_ip, in_sp, in_cycle : in std_logic_vector(num_wordbits-1 downto 0);
		in_show_sp, in_show_instr : in std_logic;

		-- ram interface
		out_ram_write : out std_logic;
		in_ram_ready : in std_logic;
		out_ram_addr : out std_logic_vector(num_addrbits-1 downto 0);
		in_ram : in std_logic_vector(num_wordbits-1 downto 0);
		
		out_disp0, out_disp1, out_disp2, out_disp3 : out std_logic_vector(3 downto 0);
		out_disp4, out_disp5 : out std_logic_vector(3 downto 0)
	);
end entity;


architecture dbgout_impl of dbgout is
begin

	-- show instruction pointer or stack pointer
	out_disp2 <= 
		in_ip(3 downto 0) when in_show_sp='0' else
		in_sp(3 downto 0) when in_show_sp='1' else
		"0000";
	out_disp3 <= 
		in_ip(7 downto 4) when in_show_sp='0' else
		in_sp(7 downto 4) when in_show_sp='1' else
		"0000";


	-- show cycle of current instruction
	out_disp4 <= in_cycle(3 downto 0);
	out_disp5 <= in_cycle(7 downto 4);


	-- display value on top of stack or current instruction
	process(in_clk) begin
		if rising_edge(in_clk) then
			out_ram_write <= '0';

			if in_show_instr='0' then
				out_ram_addr <= in_sp;
			else
				out_ram_addr <= in_ip;
			end if;
			
			if in_ram_ready='1' then
				out_disp0 <= in_ram(3 downto 0);
				out_disp1 <= in_ram(7 downto 4);
			end if;
		end if;
	end process;

end architecture;
