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
		constant num_words : natural := 2**8 --2**num_addrbits
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
end entity;



architecture ram_impl of ram is
	-- a word type
	subtype t_word is std_logic_vector(num_wordbits-1 downto 0);
	-- an array of words type
	type t_words is array(0 to num_words-1) of t_word;

	-- the memory is an array of words
	signal words : t_words := (
		x"10", x"20",  -- push function 1 address, 0x20
		x"22",         -- call function 1
		x"10", x"40",  -- push function 2 address, 0x40
		x"22",         -- call function 2
		x"10", x"ff",  -- push 0xff (value will be overwritten)

		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",

		-- 0x20: function 1, calculation test
		x"10", x"04",  -- push 4
		x"10", x"08",  -- push 8
		x"10", x"03",  -- push 3
		x"02",         -- sub
		x"01",         -- add
		x"10", x"03",  -- push 3
		x"04",         -- mul
		x"10", x"07",  -- push address 0x07 for value to overwrite
		x"11",         -- pop result
		x"20",         -- branch (return)

		x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",

		-- 0x40: function 2, loop test
		x"10", x"60",  -- push address 0x60
		x"12",         -- push value at address
		x"10", x"01",  -- push 1
		x"01",         -- add
		x"10", x"60",  -- push address 0x60
		x"11",         -- pop result
		x"10", x"60",  -- push address 0x60
		x"12",         -- push value at address
		x"10", x"0a",  -- push 10
		x"35",         -- less than
		x"3a",         -- not
		x"10", x"56",  -- push address 0x56
		x"21",         -- conditional branch
		x"10", x"40",  -- push function 2 start address, 0x40
		x"20",         -- else: loop 
		x"20",         -- 0x56: return branch

		x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",

		-- 0x60
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",

		-- 0x80
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",

		-- 0xa0
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",

		-- 0xc0
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",

		-- 0xe0
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00",
		x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00"
	);

begin
	-- generate ports 0 to num_ports-1
	--gen_ports : for portidx in 0 to num_ports-1 generate

	-- in case the for-generate loop is not available, the
	-- process has to be copied for the individual ports
	gen_port0 : if num_ports >= 0 generate
		constant portidx : natural := 0;

	begin
		-- ram
		gen_ram : if is_rom = '0' generate
			process(in_clk)
				variable curaddr : integer range 0 to num_words-1;
			begin
				if rising_edge(in_clk) then
					curaddr := to_int(in_addr(portidx));

					if in_write(portidx) = '1' then
						-- write a word to memory
						words(curaddr) <= in_word(portidx);
					end if;

					-- read a word from memory
					out_word(portidx) <= words(curaddr);
				end if;
			end process;
		end generate; -- ram

		-- rom
		gen_rom : if is_rom = '1' generate
			process(in_clk)
				variable curaddr : integer range 0 to num_words-1;
			begin
				if rising_edge(in_clk) then
					curaddr := to_int(in_addr(portidx));

					-- read a word from memory
					out_word(portidx) <= words(curaddr);
				end if;
			end process;
		end generate;	-- rom
	end generate;	-- ports



	gen_port1 : if num_ports >= 0 generate
		constant portidx : natural := 1;

	begin
		-- ram
		gen_ram : if is_rom = '0' generate
			process(in_clk)
				variable curaddr : integer range 0 to num_words-1;
			begin
				if rising_edge(in_clk) then
					curaddr := to_int(in_addr(portidx));

					--if in_write(portidx) = '1' then
					--	-- write a word to memory
					--	words(curaddr) <= in_word(portidx);
					--end if;

					-- read a word from memory
					out_word(portidx) <= words(curaddr);
				end if;
			end process;
		end generate; -- ram

		-- rom
		gen_rom : if is_rom = '1' generate
			process(in_clk)
				variable curaddr : integer range 0 to num_words-1;
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
