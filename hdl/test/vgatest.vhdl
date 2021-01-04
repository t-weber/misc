--
-- vga test (on real hardware)
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--

library ieee;
use ieee.std_logic_1164.all;


entity vgatest is
	generic
	(
		constant pixaddr_len : natural := 14;
		constant rgb_len : natural := 4;
		constant rgbword_len : natural := rgb_len*3
	);

	port(
		-- 50 MHz clock
		clk : in std_logic;

		-- vga signals
		vga_hs, vga_vs : out std_logic;
		vga_r, vga_g, vga_b : out std_logic_vector(3 downto 0);

		-- keys
		key : in std_logic_vector(0 downto 0);
		sw : in std_logic_vector(0 downto 0)
	);
end entity;


architecture vgatest_impl of vgatest is
	signal memaddr : std_logic_vector(pixaddr_len-1 downto 0);
	signal mem : std_logic_vector(rgbword_len-1 downto 0);

begin
	-- rom image
	--romimg : entity work.rom port map(in_addr=>memaddr, out_data=>mem);

	-- ram image, 200x75 pixels, 4-bits colour
	ramimg : entity work.ram
		port map(
			in_addr(0)=>memaddr, out_word(0)=>mem,
			in_clk=>clk, in_write(0)=>'0', in_word(0)=>(others=>'0')
		);

	-- vga module
	vgamod : entity work.vga
		generic map(
			num_pixaddrbits=>pixaddr_len,
			num_rgbbits=>rgbword_len,
			num_colourbits=>rgb_len
		)
		port map(
			in_rst=>not key(0), in_clk=>clk,
			in_testpattern=>sw(0),
			out_hsync=>vga_hs, out_vsync=>vga_vs,
			out_r=>vga_r, out_g=>vga_g, out_b=>vga_b,
			in_mem=>mem, out_mem_addr=>memaddr
		);
end architecture;
