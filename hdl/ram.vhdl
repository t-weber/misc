--
-- n-port ram
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date dec-2020
-- @license see 'LICENSE.EUPL' file
--
-- reference:
--	- listing 5.24 on pp. 119-120 of the book by Pong P. Chu, 2011, ISBN 978-1-118-00888-1.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


package mem is
	type t_logicarray is array(natural range <>) of std_logic;
	type t_logicvecarray is array(natural range <>) of std_logic_vector;

	component ram is
		generic(
			-- rom or ram?
			constant is_rom : in std_logic := '0';
			-- number of read/write access ports
			constant num_ports : natural := 2;

			-- bus size for addressing individual words
			constant num_addrbits : natural := 8;
			-- word size
			constant num_wordbits : natural := 8;
			-- number of stored words
			constant num_words : natural := 2**num_addrbits
		);

		port(
			in_clk : in std_logic;

			-- write to or read from the given address?
			in_write : in t_logicarray(0 to num_ports-1);
			in_addr : in t_logicvecarray(0 to num_ports-1)(num_addrbits-1 downto 0);

			-- input and output word
			in_word : in t_logicvecarray(0 to num_ports-1)(num_wordbits-1 downto 0);
			out_word : out t_logicvecarray(0 to num_ports-1)(num_wordbits-1 downto 0)
		);
	end component;
end package;


-------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.mem.t_logicarray;
use work.mem.t_logicvecarray;
use work.conv.all;


entity ram is
	generic(
		-- rom or ram?
		constant is_rom : in std_logic := '0';
		-- number of read/write access ports
		constant num_ports : natural := 2;

		-- bus size for addressing individual words
		constant num_addrbits : natural := 8;
		-- word size
		constant num_wordbits : natural := 8;
		-- number of stored words
		constant num_words : natural := 2**num_addrbits
	);

	port(
		in_clk : in std_logic;

		-- write to or read from the given address?
		in_write : in t_logicarray(0 to num_ports-1);
		in_addr : in t_logicvecarray(0 to num_ports-1)(num_addrbits-1 downto 0);

		-- input and output word
		in_word : in t_logicvecarray(0 to num_ports-1)(num_wordbits-1 downto 0);
		out_word : out t_logicvecarray(0 to num_ports-1)(num_wordbits-1 downto 0)
	);
end ram;


architecture ram_impl of ram is
	-- a word type
	subtype t_word is std_logic_vector(num_wordbits-1 downto 0);
	-- an array of words type
	type t_words is array(0 to num_words-1) of t_word;

	-- the memory is an array of words
	signal words : t_words;	-- := (x"00", x"00", x"00", x"00");

begin
	-- generate ports 0 to num_ports-1
	gen_port : for portidx in 0 to num_ports-1 generate
		-- ram
		gen_ram : if is_rom='0' generate
			process(in_clk)
				variable curaddr : integer;
			begin
				if rising_edge(in_clk) then
					curaddr := to_int(in_addr(portidx));

					-- write a word to memory
					if in_write(portidx) = '1' then
						words(curaddr) <= in_word(portidx);
					end if;

					-- read a word from memory
					out_word(portidx) <= words(curaddr);
				end if;
			end process;
		end generate; -- ram

		-- rom
		gen_rom : if is_rom='1' generate
			process(in_clk)
				variable curaddr : integer;
			begin
				if rising_edge(in_clk) then
					curaddr := to_int(in_addr(portidx));

					-- read a word from memory
					out_word(portidx) <= words(curaddr);
				end if;
			end process;
		end generate;	-- rom
	end generate;	-- ports

end architecture;
