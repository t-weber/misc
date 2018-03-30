/**
 * GIL tests
 * @author Tobias Weber
 * @date 16-dec-17
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://github.com/boostorg/gil/tree/develop/example
 *
 * g++ -o gil gil.cpp -std=c++17 -ljpeg
 */

#include <iostream>

#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
//#include <boost/gil/extension/io/png_io.hpp>
namespace gil = boost::gil;
namespace mpl = boost::mpl;


int main()
{
	using t_image = gil::rgb8_image_t;
	using t_image_gray = gil::gray8_image_t;


	// ------------------------------------------------------------------------
	// open image
	t_image img;
	try
	{
		gil::jpeg_read_image("/home/tw/tmp/I/0.jpg", img);
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// get data iterators
	auto view = gil::view(img);
	std::cout << "Dimensions: " << img.width() << "x" << img.height() << "x" << view.num_channels() << ".\n";


	// write to row
	for(auto iterRow = view.row_begin(100); iterRow != view.row_end(100); ++iterRow)
	{
		(*iterRow)[0] = 0xff;
		(*iterRow)[1] = 0xff;
		(*iterRow)[2] = 0xff;
	}

	// write to column
	for(auto iterCol = view.col_begin(100); iterCol != view.col_end(100); ++iterCol)
	{
		(*iterCol)[0] = 0xff;
		(*iterCol)[1] = 0xff;
		(*iterCol)[2] = 0xff;
	}
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// use first channel as grayscale image
	t_image_gray imgGray(img.width(), img.height());
	auto viewGray = gil::view(imgGray);

	for(std::size_t iY=0; iY < img.height(); ++iY)
	{
		auto iterRow = view.row_begin(iY);
		auto iterRowGray = viewGray.row_begin(iY);
		for(; iterRow != view.row_end(iY); ++iterRow, ++iterRowGray)
			(*iterRowGray)[0] = (*iterRow)[0];
	}
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// write image
	//gil::jpeg_write_view("/home/tw/tmp/I/1.jpg", view, 85);
	gil::jpeg_write_view("/home/tw/tmp/I/1.jpg", viewGray, 85);
	//gil::png_write_view("/home/tw/tmp/I/1.png", view);
	// ------------------------------------------------------------------------


	return 0;
}
