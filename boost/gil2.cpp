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
#include <cmath>

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
	//std::cout << "(" << x_start << ", " << y_start << ") -> (" << x_end << ", " << y_end << ")" << std::endl;

	const t_x x_range = x_end - x_start;
	const t_y y_range = y_end - y_start;
	const t_x x_range_abs = std::abs(x_range);
	const t_y y_range_abs = std::abs(y_range);

	const t_x x_inc = x_range > 0 ? 1 : -1;
	const t_y y_inc = y_range > 0 ? 1 : -1;


	auto x_in_bounds = [x_end, x_inc](t_x x) -> bool
	{
		if(x_inc > 0)
			return x <= x_end;
		return x >= x_end;
	};

	auto y_in_bounds = [y_end, y_inc](t_y y) -> bool
	{
		if(y_inc > 0)
			return y <= y_end;
		return y >= y_end;
	};


	// special cases: straight line
	if(x_range == 0)
	{
		for(t_y y=y_start; y_in_bounds(y); y+=y_inc)
			view(x_start, y) = col;
		return;
	}

	if(y_range == 0)
	{
		for(t_x x=x_start; x_in_bounds(x); x+=x_inc)
			view(x, y_start) = col;
		return;
	}


	// general case: sloped line with x range larger than y range
	if(x_range_abs >= y_range_abs)
	{
		const t_x mult = 2;
		t_y y = y_start;
		t_x err = x_range * mult/2;

		for(t_x x=x_start; x_in_bounds(x); x+=x_inc)
		{
			//std::cout << "x=" << x << ", y=" << y << ", err=" << err << std::endl;

			view(x, y) = col;
			err -= mult * y_range;

			if(err < 0 && y_inc > 0)
			{
				y += y_inc;
				err += mult * x_range_abs;
			}

			if(err > 0 && y_inc < 0)
			{
				y += y_inc;
				err -= mult * x_range_abs;
			}
		}
	}


	// general case: sloped line with y range larger than x range
	else
	{
		const t_y mult = 2;
		t_x x = x_start;
		t_y err = y_range * mult/2;

		for(t_y y=y_start; y_in_bounds(y); y+=y_inc)
		{
			//std::cout << "x=" << x << ", y=" << y << ", err=" << err << std::endl;

			view(x, y) = col;
			err -= mult * x_range;

			if(err < 0 && x_inc > 0)
			{
				x += x_inc;
				err += mult * y_range_abs;
			}

			else if(err > 0 && x_inc < 0)
			{
				x += x_inc;
				err -= mult * y_range_abs;
			}
		}
	}
}


/**
 * bresenham algorithm
 * @see https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * @see https://de.wikipedia.org/wiki/Bresenham-Algorithmus
 */
template<class t_view, class t_col = typename t_view::value_type,
	class t_x = typename t_view::x_coord_t, class t_y = typename t_view::y_coord_t>
void draw_rect(t_view& view, t_x x1, t_y y1, t_x x2, t_y y2, t_col col = t_col{0x00})
{
	draw_line(view, x1, y1, x2, y1);
	draw_line(view, x1, y1, x1, y2);
	draw_line(view, x2, y2, x1, y2);
	draw_line(view, x2, y2, x2, y1);
}


/**
 * bresenham circle algorithm
 * @see https://de.wikipedia.org/wiki/Bresenham-Algorithmus#Kreisvariante_des_Algorithmus
 */
template<class t_view, class t_col = typename t_view::value_type,
	class t_x = typename t_view::x_coord_t, class t_y = typename t_view::y_coord_t>
void draw_circle(t_view& view, t_x x_centre, t_y y_centre, t_x rad, t_col col = t_col{0x00})
{
	auto draw_all = [x_centre, y_centre, &col, &view](t_x x, t_y y)
	{
		view(x_centre + x, y_centre + y) = col;
		view(x_centre + x, y_centre - y) = col;
		view(x_centre - x, y_centre + y) = col;
		view(x_centre - x, y_centre - y) = col;
	};


	t_x x = rad;
	for(t_y y=0, err=-x; y<x; ++y, err+=2*y + 1)
	{
		if(err > 0)
		{
			err -= 2*x - 1;
			--x;
		}

		draw_all(x, y);
	}


	t_y y = rad;
	for(t_x x=0, err=-y; x<y; ++x, err+=2*x + 1)
	{
		if(err > 0)
		{
			err -= 2*y - 1;
			--y;
		}

		draw_all(x, y);
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

	double centre_x = 100.;
	double centre_y = 100.;

	double scale_x = 50.;
	double scale_y = 50.;

	for(double angle=0.; angle<2.*M_PI; angle+=M_PI/10.)
	{
		double s = std::sin(angle);
		double c = std::cos(angle);

		draw_line(view, centre_x, centre_y, centre_x + scale_x*c, centre_y + scale_y*s);
	}

	draw_circle(view, 202, 100, 50);
	draw_rect(view, 48, 48, 254, 152);

	gil::write_view("0.png", view, gil::png_tag{});

	return 0;
}
