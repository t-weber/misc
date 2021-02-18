--
-- timing test for sram module (sim)
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date feb-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 timing.vhdl  &&  ghdl -a --std=08 timing_test.vhdl  &&  ghdl -e --std=08 timing_test timing_test_impl
-- ./timing_test-timing_test_impl --stop-time=100ns --vcd=test_timing.vcd  &&  gtkwave test_timing.vcd --rcvar "do_initial_zoom_fit yes"
--

library ieee;
use ieee.std_logic_1164.all;



entity timing_test is
	generic
	(
		-- main clock
		constant CLKDELAY : time := 2.5 ns;

		-- shifted clock
		constant CLK1SHIFT : time := 1 ns;

		-- clock with modified duty cycle
		constant CLK2SHIFT : time := 0 ns;
		constant CLK2DUTY : time := 4 ns;

		-- address and data widths
		constant ADDR_WIDTH : natural := 8;
		constant DATA_WIDTH : natural := 8
	);
end entity;



architecture timing_test_impl of timing_test is
	signal theclk, theclk_shifted, theclk_mod : std_logic := '1';

	signal enable, write_enable : std_logic := '0';

	signal addr : std_logic_vector(ADDR_WIDTH-1 downto 0); 
	signal data : std_logic_vector(DATA_WIDTH-1 downto 0);
begin

	theclk <= not theclk after CLKDELAY;


	---------------------------------------------------------------------------
	-- clocks
	---------------------------------------------------------------------------
	-- shifted clock
	proc_shiftedclk : process
	begin
		wait for CLK1SHIFT;

		loop
			wait for CLKDELAY;
			theclk_shifted <= not theclk_shifted;
		end loop;
	end process;


	--
	-- clock with shorter duty cycle
	--
	proc_modclk : process begin
		loop
			wait for CLK2SHIFT;
			theclk_mod <= '1';

			wait for CLK2DUTY;
			theclk_mod <= '0';

			wait for CLKDELAY*2 - CLK2DUTY - CLK2SHIFT;
		end loop;
	end process;
	---------------------------------------------------------------------------


	---------------------------------------------------------------------------
	--
	-- test input sequence
	--
	proc_test : process begin
		-- read test
		wait for 2 ns;
		enable <= '1';
		write_enable <= '0';
		addr <= x"12";
		wait for 15 ns;
		enable <= '0';

		-- read test 2
		--wait for 5 ns;
		enable <= '1';
		write_enable <= '0';
		addr <= x"34";
		wait for 15 ns;
		enable <= '0';

		-- write test
		--wait for 10 ns;
		enable <= '1';
		write_enable <= '1';
		addr <= x"ab";
		data <= x"78";
		wait for 10 ns;
		enable <= '0';
		write_enable <= '0';
	end process;
	---------------------------------------------------------------------------


	---------------------------------------------------------------------------
	-- test module
	---------------------------------------------------------------------------
	mod_timing : entity work.timing
		generic map(
			ADDR_WIDTH => ADDR_WIDTH, DATA_WIDTH => DATA_WIDTH
		)
		port map(
			in_clk => theclk, in_clk_mod => theclk_mod,
			in_reset => '0', in_enable => enable, in_writeenable => write_enable,
			in_addr => addr, in_data => data
		);
	---------------------------------------------------------------------------
end architecture;
