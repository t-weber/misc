/**
 * GIL tests
 * @author Tobias Weber
 * @date jan-2022
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *  * https://github.com/boostorg/gil/tree/develop/example
 *
 * g++ -o gil2 gil2.cpp -std=c++20 -lpng
 */

#include <iostream>

#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/png.hpp>
namespace gil = boost::gil;


/**
 * bresenham algorithm
 * @see https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * @see https://de.wikipedia.org/wiki/Bresenham-Algorithmus
 */
template<class t_view, class t_col = typename t_view::value_type,
	class t_x = typename t_view::x_coord_t, class t_y = typename t_view::y_coord_t>
void draw_line(t_view& view, t_x x_start, t_y y_start, t_x x_end, t_y y_end, t_col col = t_col{0x00})
{
	const t_x x_range = x_end - x_start;
	const t_y y_range = y_end - y_start;

	const t_x x_inc = x_range > 0 ? 1 : -1;
	const t_y y_inc = y_range > 0 ? 1 : -1;


	// special cases: straight line
	if(x_range == 0)
	{
		for(t_y y=y_start; y!=y_end; y+=y_inc)
			view(x_start, y) = col;
		return;
	}

	if(y_range == 0)
	{
		for(t_x x=x_start; x!=x_end; x+=x_inc)
			view(x, y_start) = col;
		return;
	}


	// general case: sloped line
	const t_x mult = 2;
	t_y y = y_start;
	t_x err = x_range * mult/2;

	for(t_x x=x_start; x!=x_end; x+=x_inc)
	{
		view(x, y) = col;
		err -= mult*y_range;

		if(err < 0)
		{
			++y;
			err += mult*x_range;
		}
		else if(err > 0)
		{
			--y;
			err -= mult*x_range;
		}
	}
}


int main()
{
	using t_img = gil::gray8_image_t;
	using t_view = typename t_img::view_t;

	t_img img(320, 240);
	t_view view = gil::view(img);

	for(typename t_view::y_coord_t y=0; y < img.height(); ++y)
	{
		for(auto iter = view.row_begin(y); iter != view.row_end(y); ++iter)
			(*iter)[0] = 0xff;
	}

	draw_line(view, 100, 100, 200, 200);
	draw_line(view, 100, 200, 200, 100);
	draw_line(view, 220, 210, 120, 110);
	draw_line(view, 220, 110, 120, 210);
	draw_line(view, 50, 50, 50, 100);
	draw_line(view, 50, 50, 100, 50);

	gil::write_view("0.png", view, gil::png_tag{});

	return 0;
}
