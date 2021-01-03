--
-- vga test
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- ghdl -a --std=08 conv.vhdl  &&  ghdl -a --std=08 vga.vhdl  &&  ghdl -a --std=08 vga_test.vhdl  &&  ghdl -e --std=08 testbed thetester
-- ghdl -r --std=08 testbed thetester --vcd=vga_test.vcd
-- gtkwave vga_test.vcd
--

library ieee;
use ieee.std_logic_1164.all;


entity testbed is
	generic
	(
		constant pixaddr_len : natural := 19;
		constant rgbchan_len : natural := 8; 	-- bits per channel
		constant rgbword_len : natural := 3*rgbchan_len;

		constant thedelay : time := 100 ns
	);
end entity;


architecture thetester of testbed is
	signal theclk : std_logic := '0';

	--signal hpix, vpix : std_logic_vector(11-1 downto 0);
	signal hsync, vsync : std_logic;

	signal memaddr : std_logic_vector(pixaddr_len-1 downto 0);
	signal mem : std_logic_vector(rgbword_len-1 downto 0);
	signal r,g,b : std_logic_vector(rgbchan_len-1 downto 0);
begin
	theclk <= not theclk after thedelay;

--	romimg : entity work.rom
--		port map(
--			in_addr=>memaddr,
--			out_data=>mem
--		);

	vgamod : entity work.vga
		generic map(
			num_pixaddrbits=>pixaddr_len,
			--num_rowcolbits=>11,
			num_colourbits=>rgbchan_len,
			num_rgbbits=>rgbword_len
		)
		port map(
			in_rst=>'0', in_clk=>theclk,
			in_testpattern=>'0',
			--out_hpix=>hpix, out_vpix=>vpix,
			out_hsync=>hsync, out_vsync=>vsync,
			in_mem=>mem, out_mem_addr=>memaddr,
			out_r=>r, out_g=>g, out_b=>b
		);
end architecture;
