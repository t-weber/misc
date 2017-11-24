/**
 * geometry tests and snippets
 * @author Tobias Weber
 * @date 24-nov-17
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/libs/geometry/doc/html/index.html
 *  * https://github.com/boostorg/geometry/tree/develop/example
 *
 * gcc -o geo geo.cpp -std=c++17 -lstdc++ -lm
 */

#include <iostream>
#include <fstream>
#include <string>

#include <boost/geometry.hpp>
#include <boost/geometry/strategies/transform.hpp>
namespace geo = boost::geometry;
namespace trafo = boost::geometry::strategy::transform;


using t_real = double;
using t_vertex = geo::model::point<t_real, 2, geo::cs::cartesian>;
using t_lines = geo::model::linestring<t_vertex>;
using t_poly = geo::model::polygon<t_vertex>;
using t_svg = geo::svg_mapper<t_vertex>;
constexpr std::size_t geo_iDim = geo::traits::dimension<t_vertex>::value;
using t_trafo = trafo::matrix_transformer<t_real, geo_iDim, geo_iDim>;


int main()
{
	// points
	t_vertex pt1(1., 2.);
	t_vertex pt2(3., 9.);
	t_vertex pt3(5., 1.);
	std::cout << geo::distance(pt1, pt2) << "\n";


	// lines
	t_lines l1;
	l1.push_back(pt1);
	l1.push_back(pt2);
	l1.push_back(pt3);
	l1.push_back(pt1);
	std::cout << geo::length(l1) << "\n";


	// polys
	t_poly poly1;
	geo::convex_hull(l1, poly1);
	geo::correct(poly1);
	t_vertex ptCent;
	geo::centroid(poly1, ptCent);
	std::cout << geo::area(poly1) << "\n";


	// intersections
	t_lines l2;
	l2.push_back(t_vertex(0., 0.));
	l2.push_back(t_vertex(10., 10.));
	std::vector<t_vertex> vecPts;
	geo::intersection(poly1, l2, vecPts);


	// trafos
	t_lines l3;
	// matrix in homogeneous coordinates
	t_trafo trafo1(1.,0.,2., 0.,1.,0., 0.,0.,0.);
	geo::transform(l2, l3, trafo1);


	// svg
	std::ofstream ofstr("tst.svg");
	t_svg svg1(ofstr, 100, 100, "width=\"200px\" height=\"200px\"");

	svg1.add(poly1);
	svg1.map(poly1, "stroke:#000000; stroke-width:1px; fill:none; stroke-linecap:round; stroke-linejoin:round;", 1.);

	svg1.add(l2);
	svg1.map(l2, "stroke:#000000; stroke-width:1px; fill:none; stroke-linecap:round; stroke-linejoin:round;", 1.);

	svg1.add(l3);
	svg1.map(l3, "stroke:#000000; stroke-width:1px; fill:none; stroke-linecap:round; stroke-linejoin:round;", 1.);

	for(const auto& vert : vecPts)
	{
		svg1.add(vert);
		svg1.map(vert, "stroke:#0000ff; stroke-width:1px; fill:#0000ff;", 1.);

		svg1.text(vert, "intersection", "font-family:\'DejaVu Sans\'; font-size:6pt", 2.,2., 8.);
	}

	svg1.add(pt1);
	svg1.add(pt2);
	svg1.add(pt3);
	svg1.add(ptCent);
	svg1.map(pt1, "stroke:#000000; stroke-width:1px; fill:#000000;", 1.);
	svg1.map(pt2, "stroke:#000000; stroke-width:1px; fill:#000000;", 1.);
	svg1.map(pt3, "stroke:#000000; stroke-width:1px; fill:#000000;", 1.);
	svg1.map(ptCent, "stroke:#ff0000; stroke-width:1px; fill:#ff0000;", 1.);

	return 0;
}
