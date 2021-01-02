--
-- vga
-- @author Tobias Weber <tobias.weber@tum.de>
-- @date jan-2021
-- @license see 'LICENSE.EUPL' file
--
-- references:
--	https://www.digikey.com/eewiki/pages/viewpage.action?pageId=15925278
--	https://academic.csuohio.edu/chu_p/rtl/sopc_vhdl.html
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.conv.all;


entity vga is
	generic(
		constant pixaddr_len : natural := 11;

		constant hpix_visible : natural := 800;
		constant vpix_visible : natural := 600;

		constant hsync_startpix : natural := hpix_visible + 56;
		constant hsync_stoppix : natural := hsync_startpix + 120;
		constant hpix_total : natural := hsync_stoppix + 64;

		constant vsync_startpix : natural := vpix_visible + 37;
		constant vsync_stoppix : natural := vsync_startpix + 6;
		constant vpix_total : natural := vsync_stoppix + 23
	);

	port(
		in_clk, in_rst : in std_logic;
		out_hpix, out_vpix : out std_logic_vector(pixaddr_len-1 downto 0);
		out_hsync, out_vsync : out std_logic
	);
end entity;


architecture vga_impl of vga is
	signal h_ctr, h_ctr_next : natural := 0;
	signal v_ctr, v_ctr_next : natural := 0;
	signal h_ctr_fin : std_logic := '0';
begin
	hctr_proc : process(in_clk, in_rst) begin
		if in_rst='1' then
			h_ctr <= 0;
		elsif rising_edge(in_clk) then
			h_ctr <= h_ctr_next;
		end if;
	end process;

	vctr_proc : process(h_ctr_fin, in_rst) begin
		if in_rst='1' then
			v_ctr <= 0;
		elsif rising_edge(h_ctr_fin) then
			v_ctr <= v_ctr_next;
		end if;
	end process;

	-- next pixel / line
	h_ctr_next <= 0 when h_ctr=hpix_total-1 else h_ctr+1;
	v_ctr_next <= 0 when v_ctr=vpix_total-1 else v_ctr+1;

	-- line finished
	h_ctr_fin <= '1' when h_ctr=hpix_total-1 else '0';

	-- current pixel
	out_hpix <= nat_to_logvec(h_ctr, out_hpix'length);
	out_vpix <= nat_to_logvec(v_ctr, out_vpix'length);

	-- synchronisation signals
	out_hsync <= '0' when h_ctr>=hsync_startpix and h_ctr<hsync_stoppix else '1';
	out_vsync <= '0' when v_ctr>=vsync_startpix and v_ctr<vsync_stoppix else '1';
end architecture;
