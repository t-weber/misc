/**
 * writes a vhdl rom image from a jpg file (8 bits per channel)
 * @author Tobias Weber
 * @date dec-2020
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://github.com/boostorg/gil/tree/develop/example
 *
 * g++ -std=c++20 -Wall -Wextra -o jpgrom jpgrom.cpp -ljpeg
 */

#include <iostream>
#include <fstream>
#include <iomanip>

#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg.hpp>
//#include <boost/gil/extension/io/png.hpp>
namespace gil = boost::gil;
namespace mpl = boost::mpl;


bool write_rom(const char* jpgfile)
{
	using t_image = gil::rgb8_image_t;

	try
	{
		// input stream
		std::ifstream istrJpg{jpgfile};
		// output stream
		std::ostream& ostrRom = std::cout;

		// open image
		t_image img;
		gil::read_image(istrJpg, img, gil::image_read_settings<gil::jpeg_tag>{});

		// get data iterators
		auto view = gil::view(img);
		std::size_t numwords = img.width()*img.height();
		unsigned neededbits = unsigned(std::ceil(std::log2(double(numwords))));

		std::cerr << "Image dimensions: " << img.width() << " x " << img.height() << " x " << view.num_channels() << "." << std::endl;
		std::cerr << "Needed bits to address: " << neededbits << "." << std::endl;

		ostrRom << R"STR(library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std;

entity rom is
	port(
		in_addr : in std_logic_vector()STR";

		ostrRom << neededbits-1;

		ostrRom << R"STR( downto 0);
		out_data : out std_logic_vector(23 downto 0)
	);
end entity;

architecture rom_impl of rom is
)STR";

		ostrRom << "\tsubtype t_rgb is std_logic_vector(23 downto 0);\n"
			<< "\ttype t_img is array(0 to " << numwords-1 << ") of t_rgb;\n"
			<< "\n\tconstant img : t_img := (\n";

		for(long iY=0; iY<img.height(); ++iY)
		{
			ostrRom << "\t\t";

			auto iterRow = view.row_begin(iY);
			for(; iterRow != view.row_end(iY); ++iterRow)
			{
				ostrRom << "x\"";
				for(std::size_t chan=0; chan<3; ++chan)
				{
					std::size_t ch = chan;
					if(ch >= view.num_channels())
						ch = 0;
					ostrRom << std::hex << std::setfill('0') << std::setw(2)
						<< static_cast<unsigned>((*iterRow)[ch]);
				}
				ostrRom << "\"";

				if(iY+1<img.height() || std::next(iterRow, 1)!=view.row_end(iY))
					ostrRom << ", ";
			}

			ostrRom << "\n";
		}

		ostrRom << "\t);\n";

		ostrRom <<
R"STR(
	function to_int(vec : std_logic_vector) return integer is
	begin
		return numeric_std.to_integer(numeric_std.unsigned(vec));
	end function;

begin
	out_data <= img(to_int(in_addr));
end architecture;
)STR";
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return false;
	}

	return true;
}


int main(int argc, char** argv)
{
	if(argc <= 1)
	{
		std::cerr << "Please give a jpg file name." << std::endl;
		return -1;
	}

	if(!write_rom(argv[1]))
		return -1;
	return 0;
}
