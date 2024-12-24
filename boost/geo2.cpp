/**
 * svg scaling test
 * @author Tobias Weber
 * @date 24-dec-24
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/libs/geometry/doc/html/index.html
 *  * http://www.boost.org/doc/libs/1_65_1/libs/geometry/doc/html/geometry/spatial_indexes/rtree_examples.html
 *  * https://www.boost.org/doc/libs/1_76_0/libs/geometry/doc/html/geometry/reference/algorithms/buffer/buffer_7_with_strategies.html
 *  * https://github.com/boostorg/geometry/tree/develop/example
 *
 * g++ -std=c++20 -o geo2 geo2.cpp
 */

#include <iostream>
#include <fstream>

#include <boost/function_output_iterator.hpp>
#include <boost/geometry.hpp>
namespace geo = boost::geometry;


using t_real = double;

template<class T = t_real> using t_vertex = geo::model::point<T, 2, geo::cs::cartesian>;
template<class T = t_real> using t_lines = geo::model::linestring<t_vertex<T>, std::vector>;
template<class T = t_real> using t_poly = geo::model::polygon<t_vertex<T>,
	false /*cw*/, false /*closed*/, std::vector>;
template<class T = t_real> using t_box = geo::model::box<t_vertex<T>>;

template<class T = t_real> using t_svg = geo::svg_mapper<t_vertex<T>,
	true /* equal scale*/, t_real>;


int main()
{
	t_real min_x = -5.;
	t_real max_x =  5.;
	t_real min_y = -5.;
	t_real max_y =  5.;

	// points
	t_vertex<t_real> pt1{ min_x, min_y };
	t_vertex<t_real> pt2{ max_x, min_y };
	t_vertex<t_real> pt3{ max_x, max_y };
	t_vertex<t_real> pt4{ min_x, max_y };

	// lines
	t_lines<t_real> l1;
	l1.push_back(pt1);
	l1.push_back(pt2);
	l1.push_back(pt3);
	l1.push_back(pt4);
	l1.push_back(pt1);

	// polys
	t_poly<t_real> poly1;
	poly1.outer().push_back(pt1);
	poly1.outer().push_back(pt2);
	poly1.outer().push_back(pt3);
	poly1.outer().push_back(pt4);


	// svg
	std::ofstream ofstr("tst.svg");
	t_svg<t_real> svg1(ofstr, 100, 100, // visible area
		"width=\"100px\" height=\"100px\"");


	// bounding box
	t_box<t_real> box1;
	box1.min_corner() = t_vertex<t_real>{ min_x, min_y };
	box1.max_corner() = t_vertex<t_real>{ max_x, max_y };
	svg1.add(box1);
	//svg1.map(box1, "stroke:#000000; stroke-width:1px; fill:none;", 1.);


	// map objects
	svg1.map(poly1, "stroke:#eeeeee; stroke-width:4px; fill:none;", 1.);
	svg1.map(l1, "stroke:#000000; stroke-width:1px; fill:none;", 1.);

	svg1.map(pt1, "stroke:#000000; stroke-width:2px; fill:#000000;", 1.);
	svg1.map(pt2, "stroke:#000000; stroke-width:2px; fill:#000000;", 1.);
	svg1.map(pt3, "stroke:#000000; stroke-width:2px; fill:#000000;", 1.);
	svg1.map(pt4, "stroke:#000000; stroke-width:2px; fill:#000000;", 1.);

	// point outside visible area
	t_vertex<t_real> pt_out{ max_x * 2., max_y * 2. };
	svg1.map(pt_out, "stroke:#ff0000; stroke-width:2px; fill:#ff0000;", 1.);

	return 0;
}
