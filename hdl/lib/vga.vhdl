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
		constant num_pixaddrbits : natural := 19;
		--constant num_rowcolbits : natural := 11;

		-- colour channels
		-- number of bits in one colour channel
		constant num_colourbits : natural := 8;
		-- number of bits in all colour channels
		constant num_rgbbits : natural := 3*num_colourbits;

		-- rows
		constant hpix_visible : natural := 800;
		constant hsync_startpix : natural := hpix_visible + 56;
		constant hsync_stoppix : natural := hsync_startpix + 120;
		constant hpix_total : natural := hsync_stoppix + 64;

		-- columns
		constant vpix_visible : natural := 600;
		constant vsync_startpix : natural := vpix_visible + 37;
		constant vsync_stoppix : natural := vsync_startpix + 6;
		constant vpix_total : natural := vsync_stoppix + 23;

        -- start address of the display buffer in memory
        constant mem_start_addr : natural := 0
	);

	port(
		-- 50 MHz clock, reset
		in_clk, in_rst : in std_logic;

		-- show test pattern?
		in_testpattern : in std_logic;

		-- vga interface
		--out_hpix, out_vpix : out std_logic_vector(num_rowcolbits-1 downto 0);
		out_hsync, out_vsync : out std_logic;
		out_r, out_g, out_b : out std_logic_vector(num_colourbits-1 downto 0);
		
		-- memory interface
		out_mem_addr : out std_logic_vector(num_pixaddrbits-1 downto 0);
		in_mem : in std_logic_vector(num_rgbbits-1 downto 0)
	);
end entity;


architecture vga_impl of vga is
	signal h_ctr, h_ctr_next : natural := 0;
	signal v_ctr, v_ctr_next : natural := 0;
	signal h_ctr_fin : std_logic := '0';

	signal output_pixel : std_logic := '0';
begin
	-- row
	hctr_proc : process(in_clk, in_rst) begin
		if in_rst='1' then
			h_ctr <= 0;
		elsif rising_edge(in_clk) then
			h_ctr <= h_ctr_next;
		end if;
	end process;

	-- column
	vctr_proc : process(h_ctr_fin, in_rst) begin
		if in_rst='1' then
			v_ctr <= 0;
		elsif rising_edge(h_ctr_fin) then
			v_ctr <= v_ctr_next;
		end if;
	end process;

	-- pixel output
	pixel_proc : process(in_clk, in_rst) begin

		if in_rst = '1' then
			-- reset
			out_r <= (others => '0');
			out_g <= (others => '0');
			out_b <= (others => '0');
	
		elsif rising_edge(in_clk) then
			if output_pixel='1' then
				-- inside visible pixel range
	
				if in_testpattern='1' then
					-- output test pattern for debugging
					if v_ctr>=0 and v_ctr<vpix_visible/2 and
						h_ctr>=0 and h_ctr<hpix_visible/3 then
						out_r <= (others => '1');
						out_g <= (others => '0');
						out_b <= (others => '0');
					elsif v_ctr>=vpix_visible/2 and v_ctr<vpix_visible and
						h_ctr>=0 and h_ctr<hpix_visible/3 then
						out_r <= (others => '0');
						out_g <= (others => '1');
						out_b <= (others => '0');
					elsif v_ctr>=0 and v_ctr<vpix_visible/2 and
						h_ctr>=hpix_visible/3 and h_ctr<2*hpix_visible/3 then
						out_r <= (others => '0');
						out_g <= (others => '0');
						out_b <= (others => '1');
					elsif v_ctr>=vpix_visible/2 and v_ctr<vpix_visible and
						h_ctr>=hpix_visible/3 and h_ctr<2*hpix_visible/3 then
						out_r <= (others => '1');
						out_g <= (others => '1');
						out_b <= (others => '0');
					elsif v_ctr>=0 and v_ctr<vpix_visible/2 and
						h_ctr>=2*hpix_visible/3 and h_ctr<hpix_visible then
						out_r <= (others => '0');
						out_g <= (others => '1');
						out_b <= (others => '1');
					elsif v_ctr>=vpix_visible/2 and v_ctr<vpix_visible and
						h_ctr>=2*hpix_visible/3 and h_ctr<hpix_visible then
						out_r <= (others => '1');
						out_g <= (others => '0');
						out_b <= (others => '1');
					end if;

				else
					-- output video memory (h_ctr+1 because of 1 cycle delay for ram!)
					out_mem_addr <= 
						nat_to_logvec(v_ctr*hpix_visible + h_ctr + mem_start_addr,
						--nat_to_logvec(v_ctr/4/2*hpix_visible/4 + (h_ctr+1)/4 + mem_start_addr,
							num_pixaddrbits);
					out_r <= in_mem(num_rgbbits-1 downto num_rgbbits-num_colourbits);
					out_g <= in_mem(num_rgbbits-num_colourbits-1 downto num_rgbbits-2*num_colourbits);
					out_b <= in_mem(num_rgbbits-2*num_colourbits-1 downto num_rgbbits-3*num_colourbits);
				end if;

			else
				-- outside visible pixel range
				out_r <= (others => '0');
				out_g <= (others => '0');
				out_b <= (others => '0');
			end if;
		end if;
	end process;

	-- next pixel / line
	h_ctr_next <= 0 when h_ctr=hpix_total-1 else h_ctr+1;
	v_ctr_next <= 0 when v_ctr=vpix_total-1 else v_ctr+1;

	-- line finished
	h_ctr_fin <= '1' when h_ctr=hpix_total-1 else '0';

	-- current pixel
	--out_hpix <= nat_to_logvec(h_ctr, out_hpix'length);
	--out_vpix <= nat_to_logvec(v_ctr, out_vpix'length);

	-- synchronisation signals
	out_hsync <= '1' when h_ctr>=hsync_startpix and h_ctr<hsync_stoppix else '0';
	out_vsync <= '1' when v_ctr>=vsync_startpix and v_ctr<vsync_stoppix else '0';

	-- output pixel when in visible region
	output_pixel <= '1' when 
		h_ctr>=0 and h_ctr<hpix_visible and
		v_ctr>=0 and v_ctr<vpix_visible
		else '0';
end architecture;
