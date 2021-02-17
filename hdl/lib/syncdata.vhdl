--
-- synchronise data across clock domains
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- @see https://www.edn.com/synchronizer-techniques-for-multi-clock-domain-socs-fpgas/
--

library ieee;
use ieee.std_logic_1164.all;


entity syncvec is
	generic(
		-- number of synchronisation steps
		constant NUM_STEPS : natural := 3;

		-- data bus length
		constant NUM_BITS : natural := 4
	);

	port(
		-- source and destination clock
		in_clk_src, in_clk_dst : in std_logic;

		-- enable signal
		in_enable : in std_logic;

		-- input data
		in_sig : in std_logic_vector(NUM_BITS-1 downto 0);

		-- output data
		out_sig : out std_logic_vector(NUM_BITS-1 downto 0)
	);
end entity;



--
-- mux synchroniser from fast to slow clock domain
-- implements the design in Fig.  9 from https://www.edn.com/synchronizer-techniques-for-multi-clock-domain-socs-fpgas/
--
architecture syncmux_impl of syncvec is
	signal enablereg : std_logic;
	signal datareg_src, datareg_dst : std_logic_vector(NUM_BITS-1 downto 0);
	signal shiftreg : std_logic_vector(0 to NUM_STEPS-1);
begin
	---------------------------------------------------------------------------
	-- source clock domain
	---------------------------------------------------------------------------
	dataproc_in : process(in_clk_src)
	begin
		if rising_edge(in_clk_src) then
			datareg_src <= in_sig;
		end if;
	end process;

	
	enableproc_in : process(in_clk_src)
	begin
		if rising_edge(in_clk_src) then
			enablereg <= in_enable;
		end if;
	end process;
	----------------------------------------------------------------------------


	---------------------------------------------------------------------------
	-- destination clock domain
	---------------------------------------------------------------------------
	-- pass signal over several flip flops
	sync_proc : process(in_clk_dst)
	begin
		if rising_edge(in_clk_dst) then
			shiftreg(0) <= enablereg;

			shiftloop: for i in 1 to NUM_STEPS-1 loop
				shiftreg(i) <= shiftreg(i-1);
			end loop;
		end if;
	end process;

	
	dataproc_out : process(in_clk_dst)
	begin
		if rising_edge(in_clk_dst) then
			if shiftreg(NUM_STEPS-1)='1' then
				datareg_dst <= datareg_src;
			end if;
		end if;
	end process;


	-- output
	out_sig <= datareg_dst;
	----------------------------------------------------------------------------

end architecture;



--
-- synchroniser from slow to fast clock domain
--
architecture syncvec_impl of syncvec is
	signal inreg : std_logic_vector(NUM_BITS-1 downto 0);

	type t_arr is array (natural range 0 to NUM_STEPS-1) of std_logic_vector(NUM_BITS-1 downto 0);
	signal shiftreg : t_arr;
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
