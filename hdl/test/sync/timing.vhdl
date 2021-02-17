--
-- timing test for sram module
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date feb-2021
-- @license see 'LICENSE.EUPL' file
--
-- @see P. P. Chu, ISBN 978-0-470-18531-5 (2008), Ch. 10, pp. 215-241,
--	https://doi.org/10.1002/9780470231630
--

library ieee;
use ieee.std_logic_1164.all;



entity timing is
	generic
	(
		-- address and data bus size
		constant ADDR_WIDTH : natural := 8;
		constant DATA_WIDTH : natural := 8;

		-- are the sram enables active high or low?
		constant ENABLES_ACTIVE_HIGH : std_logic := '1'
	);

	port
	(
		-- main clock and clock with higher duty-cycle using pll
		in_clk, in_clk_mod : in std_logic;
		-- reset
		in_reset : in std_logic;

		-- chip enable (starts read or write operation)
		in_enable : in std_logic;
		-- choose writing or reading
		in_writeenable : in std_logic;

		-- address
		in_addr : in std_logic_vector(ADDR_WIDTH-1 downto 0); 

		-- data writing and reading
		in_data : in std_logic_vector(DATA_WIDTH-1 downto 0);
		out_data : out std_logic_vector(DATA_WIDTH-1 downto 0);

		---------------------------------------------------------------
		-- sram interface
		---------------------------------------------------------------
		out_sram_readenable : out std_logic;
		out_sram_writeenable : out std_logic;

		out_sram_addr : out std_logic_vector(ADDR_WIDTH-1 downto 0);
		inout_sram_data : inout std_logic_vector(DATA_WIDTH-1 downto 0)
		---------------------------------------------------------------
	);
end entity;



architecture timing_impl of timing is
	-- states
	type t_state is (ReadOrWrite, Reading, Writing);
	signal state, state_next : t_state := ReadOrWrite;

	-- enable signal registers
	signal read_enable, read_enable_next : std_logic;
	signal write_enable, write_enable_next : std_logic;

	-- address and data registers
	signal addr, addr_next : std_logic_vector(ADDR_WIDTH-1 downto 0); 
	signal data, data_next : std_logic_vector(DATA_WIDTH-1 downto 0);

begin

	---------------------------------------------------------------------------
	-- outputs
	---------------------------------------------------------------------------
	-- cut off the last half-clock of the enable signals (needed for sram timing)
	gen_enables : if ENABLES_ACTIVE_HIGH = '1'
	-- active high
	generate
		out_sram_readenable <=
			(read_enable and in_clk_mod) when state=ReadOrWrite --state_next=Reading or state_next=Writing
			else read_enable;
		out_sram_writeenable <=
			(write_enable and in_clk_mod) when state=ReadOrWrite -- state_next=Writing or state_next=Reading
			else write_enable;

	-- active low
	else generate
		out_sram_readenable <=
			not (read_enable and in_clk_mod) when state=ReadOrWrite
			else not read_enable;
		out_sram_writeenable <=
			not (write_enable and in_clk_mod) when state=ReadOrWrite
			else not write_enable;
	end generate;

	-- address and data bus
	out_sram_addr <= addr;
	inout_sram_data <= data when write_enable='1' else (others=>'Z');
	---------------------------------------------------------------------------


	---------------------------------------------------------------------------
	-- clock process
	---------------------------------------------------------------------------
	proc_clk : process(in_clk, in_reset) is
	begin
		-- clock
		if rising_edge(in_clk) then
			-- synchronous reset
			if in_reset = '1' then
				addr <= (others => '0');
				data <= (others => '0');
				read_enable <= '0';
				write_enable <= '0';
				state <= ReadOrWrite;

			-- advance states
			else
				addr <= addr_next;
				data <= data_next;
				read_enable <= read_enable_next;
				write_enable <= write_enable_next;
				state <= state_next;
			end if;
		end if;
	end process;
	---------------------------------------------------------------------------


	---------------------------------------------------------------------------
	-- state machine for waveform generation
	---------------------------------------------------------------------------
	proc_waves : process(state, in_enable, in_writeenable,
		in_addr, in_data, inout_sram_data,
		read_enable, write_enable,
		addr, data) is
	begin

		-- save registers into next cycle
		addr_next <= addr;
		data_next <= data;
		read_enable_next <= read_enable;
		write_enable_next <= write_enable;
		state_next <= state;

		case state is
			--
			-- first clock cycle
			--
			when ReadOrWrite => null;
				if in_enable='1' then
					if in_writeenable='1' then
						addr_next <= in_addr;
						data_next <= in_data;
						write_enable_next <= '1';
						state_next <= Writing;
					else
						addr_next <= in_addr;
						read_enable_next <= '1';
						state_next <= Reading;
					end if;
				end if;

			--
			-- second clock cycle
			--
			when Reading =>
				out_data <= inout_sram_data;
				state_next <= ReadOrWrite;

			--
			-- second clock cycle
			--
			when Writing =>
				state_next <= ReadOrWrite;
		end case;
	end process;
	---------------------------------------------------------------------------

end architecture;
