--
-- synchronise across clock domains
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- @see https://www.edn.com/synchronizer-techniques-for-multi-clock-domain-socs-fpgas/
--

library ieee;
use ieee.std_logic_1164.all;


entity sync is
	generic(
		-- number of synchronisation steps
		constant NUM_STEPS : natural := 3
	);

	port(
		-- source and destination clock
		in_clk_src, in_clk_dst : in std_logic;

		-- input signal
		in_sig : in std_logic;

		-- output signal
		out_sig : out std_logic
	);
end entity;


--
-- toggle synchroniser from fast to slow clock domain
-- implements the design in Fig. 3 from https://www.edn.com/synchronizer-techniques-for-multi-clock-domain-socs-fpgas/
-- @see also https://gist.github.com/gergoerdi/b926408e1a7a991f1031#file-togglesync-vhdl
--
architecture togglesync_impl of sync is
	signal inreg : std_logic;
	signal shiftreg : std_logic_vector(0 to NUM_STEPS-1);
begin

	---------------------------------------------------------------------------
	-- source clock domain
	---------------------------------------------------------------------------
	in_proc : process(in_clk_src)
	begin
		if rising_edge(in_clk_src) then
			-- in_sig is input signal for a not gate
			case in_sig is
				when '1' => inreg <= not inreg;
				when others => null;
			end case;
		end if;
	end process;
	---------------------------------------------------------------------------


	---------------------------------------------------------------------------
	-- destination clock domain
	---------------------------------------------------------------------------
	-- pass signal over several flip flops
	sync_proc : process(in_clk_dst)
	begin
		if rising_edge(in_clk_dst) then
			shiftreg(0) <= inreg;

			shiftloop: for i in 1 to NUM_STEPS-1 loop
				shiftreg(i) <= shiftreg(i-1);
			end loop;
		end if;
	end process;


	-- output
	out_sig <= shiftreg(NUM_STEPS-1) xor shiftreg(NUM_STEPS-2);
	---------------------------------------------------------------------------

end architecture;



--
-- synchroniser from slow to fast clock domain
--
architecture sync_impl of sync is
	signal inreg : std_logic;
	signal shiftreg : std_logic_vector(0 to NUM_STEPS-1);
begin

	---------------------------------------------------------------------------
	-- source clock domain
	---------------------------------------------------------------------------
	in_proc : process(in_clk_src)
	begin
		if rising_edge(in_clk_src) then
			inreg <= in_sig;
		end if;
	end process;
	---------------------------------------------------------------------------


	---------------------------------------------------------------------------
	-- destination clock domain
	---------------------------------------------------------------------------
	-- pass signal over several flip flops
	sync_proc : process(in_clk_dst)
	begin
		if rising_edge(in_clk_dst) then
			shiftreg(0) <= inreg;

			shiftloop: for i in 1 to NUM_STEPS-1 loop
				shiftreg(i) <= shiftreg(i-1);
			end loop;
		end if;
	end process;


	-- output
	out_sig <= shiftreg(NUM_STEPS-1);
	---------------------------------------------------------------------------

end architecture;
