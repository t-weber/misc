/**
 * geometry tests and snippets
 * @author Tobias Weber
 * @date 24-nov-17
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/libs/geometry/doc/html/index.html
 *  * http://www.boost.org/doc/libs/1_65_1/libs/geometry/doc/html/geometry/spatial_indexes/rtree_examples.html
 *  * https://github.com/boostorg/geometry/tree/develop/example
 *
 * gcc -o geo geo.cpp -std=c++17 -lstdc++ -lm
 */

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

#include <boost/function_output_iterator.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/strategies/transform.hpp>
#include <boost/geometry/index/rtree.hpp>
namespace geo = boost::geometry;
namespace trafo = geo::strategy::transform;
namespace geoidx = geo::index;


using t_real = double;

template<class T = t_real>
using t_vertex = geo::model::point<T, 2, geo::cs::cartesian>;

template<class T = t_real>
constexpr std::size_t g_iDim = geo::traits::dimension<t_vertex<T>>::value;

template<class T = t_real> using t_lines = geo::model::linestring<t_vertex<T>>;
template<class T = t_real> using t_poly = geo::model::polygon<t_vertex<T>>;
template<class T = t_real> using t_svg = geo::svg_mapper<t_vertex<T>>;
template<class T = t_real> using t_trafo = trafo::matrix_transformer<T, g_iDim<T>, g_iDim<T>>;

template<class T = t_real>
using t_rtree = geoidx::rtree<std::tuple<t_vertex<T>, std::size_t, void*>, geoidx::dynamic_rstar>;


int main()
{
	// points
	t_vertex<t_real> pt1(1., 2.);
	t_vertex<t_real> pt2(3., 9.);
	t_vertex<t_real> pt3(5., 1.);
	std::cout << geo::distance(pt1, pt2) << "\n";


	// lines
	t_lines<t_real> l1;
	l1.push_back(pt1);
	l1.push_back(pt2);
	l1.push_back(pt3);
	l1.push_back(pt1);
	std::cout << geo::length(l1) << "\n";


	// polys
	t_poly<t_real> poly1;
	geo::convex_hull(l1, poly1);
	geo::correct(poly1);
	t_vertex<t_real> ptCent;
	geo::centroid(poly1, ptCent);
	std::cout << geo::area(poly1) << "\n";


	// intersections
	t_lines<t_real> l2;
	l2.push_back(t_vertex<t_real>(0., 0.));
	l2.push_back(t_vertex<t_real>(10., 10.));
	std::vector<t_vertex<t_real>> vecPts;
	geo::intersection(poly1, l2, vecPts);


	// trafos
	t_lines<t_real> l3;
	// matrix in homogeneous coordinates
	t_trafo<t_real> trafo1(1.,0.,2., 0.,1.,0., 0.,0.,0.);
	geo::transform(l2, l3, trafo1);


	// svg
	std::ofstream ofstr("tst.svg");
	t_svg<t_real> svg1(ofstr, 100, 100, "width=\"200px\" height=\"200px\"");

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


	// spatial indices
	t_rtree<t_real> rt1(typename t_rtree<t_real>::parameters_type(8));
	rt1.insert(std::make_tuple(pt1, 1, nullptr));
	rt1.insert(std::make_tuple(pt2, 2, nullptr));
	rt1.insert(std::make_tuple(pt3, 3, nullptr));

	// query nearest 2 points
	std::vector<typename t_rtree<t_real>::value_type> vecNearest;
	rt1.query(geoidx::nearest(t_vertex<t_real>(1., 3.), 2), std::back_inserter(vecNearest));
	std::cout << "nearest point indices: ";
	for(const auto& ptNearest : vecNearest)
		std::cout << std::get<1>(ptNearest) << " ";
	std::cout << "\n";

	// query nearest point, calling a lambda function
	rt1.query(geoidx::nearest(t_vertex<t_real>(1., 3.), 1),
		boost::make_function_output_iterator([](const auto& tup)
		{
			std::cout << "nearest index: " << std::get<1>(tup) << "\n";
		}));


	return 0;
}
