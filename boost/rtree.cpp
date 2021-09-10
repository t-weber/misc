/**
 * geometry tests and snippets
 * @author Tobias Weber
 * @date sep-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/libs/geometry/doc/html/index.html
 *  * http://www.boost.org/doc/libs/1_65_1/libs/geometry/doc/html/geometry/spatial_indexes/rtree_examples.html
 *  * https://www.boost.org/doc/libs/1_76_0/libs/geometry/doc/html/geometry/reference/algorithms/buffer/buffer_7_with_strategies.html
 *  * https://github.com/boostorg/geometry/tree/develop/example
 *  * https://github.com/boostorg/geometry/blob/develop/include/boost/geometry/index/detail/rtree/visitors/iterator.hpp
 *  * https://github.com/boostorg/geometry/blob/develop/include/boost/geometry/index/detail/rtree/node/weak_visitor.hpp
 *
 * g++ -Wall -Wextra -Weffc++ -std=c++20 -o rtree rtree.cpp
 */

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

// rtree with access to private members
#include </home/tw/Projects/my_rtree.hpp>
//#include <boost/geometry/index/rtree.hpp>

#include <boost/function_output_iterator.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/strategies/transform.hpp>
#include <boost/type_index.hpp>

namespace geo = boost::geometry;
namespace trafo = geo::strategy::transform;
namespace geoidx = geo::index;
namespace strat = geo::strategy::buffer;
namespace ty = boost::typeindex;


using t_real = double;

template<class T = t_real>
using t_vertex = geo::model::point<T, 2, geo::cs::cartesian>;

template<class T = t_real>
constexpr std::size_t g_iDim = geo::traits::dimension<t_vertex<T>>::value;
template<class T = t_real> using t_box = geo::model::box<t_vertex<T>>;
template<class T = t_real> using t_svg = geo::svg_mapper<t_vertex<T>>;

template<class T = t_real>
using t_rtree_value = std::tuple<t_vertex<T>, std::size_t, void*>;

template<class T = t_real>
using t_rtree = geoidx::rtree<t_rtree_value<T>, geoidx::dynamic_rstar>;
template<class T = t_real> using t_rtreebox = typename t_rtree<t_real>::bounds_type;


/*class Rtree : public t_rtree<t_real>
{
public:
	using t_rtree<t_real>::t_rtree;
	using t_members = t_rtree<t_real>::members_holder;
};*/


template<class t_tree, class t_real = double>
struct Visitor
{
	using t_bounds = typename t_tree::bounds_type;
	std::vector<t_bounds> all_bounds{};


	template<class T> void operator()(const T& node_or_leaf)
	{
		/*std::cout << "In visitor with type: "
			<< ty::type_id_with_cvr<T>().pretty_name()
			<< std::endl;*/

		const auto& elements = geoidx::detail::rtree::elements(node_or_leaf);
		constexpr bool is_node = !std::is_same_v<
			std::decay_t<decltype(*elements.begin())>, t_rtree_value<t_real>>;

		std::cout << "Elements size: " << elements.size() << std::endl;

		// are we at node level (not at the leaves)
		if constexpr(is_node)
		{
			const t_bounds& bounds = geoidx::detail::rtree::elements_box<t_bounds>(
				elements.begin(), elements.end(), m_tree->translator(), geoidx::detail::get_strategy(m_tree->parameters()));
			all_bounds.push_back(bounds);

			for(const auto& element : elements)
				geoidx::detail::rtree::apply_visitor(*this, *element.second);
		}
	}


	const t_tree* m_tree{};
};


int main()
{
	std::cout << "r-tree bounding box type: "
		<< ty::type_id_with_cvr<t_rtreebox<t_real>>().pretty_name()
		<< std::endl;
	std::cout << "r-tree value type: "
		<< ty::type_id_with_cvr<typename t_rtree<t_real>::value_type>().pretty_name()
		<< std::endl;

	/*std::cout << "r-tree internal node type: "
		<< ty::type_id_with_cvr<typename t_rtree<t_real>::members_holder::node>().pretty_name()
		<< std::endl;
	std::cout << "r-tree internal leaf type: "
		<< ty::type_id_with_cvr<typename t_rtree<t_real>::members_holder::leaf>().pretty_name()
		<< std::endl;*/


	// points
	std::vector<t_vertex<t_real>> points{{
		t_vertex<t_real>{1., 2.},
		t_vertex<t_real>{5., 8.},
		t_vertex<t_real>{7., 4.},
		t_vertex<t_real>{10., 8.},
		t_vertex<t_real>{10., 3.}
	}};


	// spatial indices
	t_rtree<t_real> rt1(typename t_rtree<t_real>::parameters_type(2));

	// insert points
	for(const auto& pt : points)
		rt1.insert(std::make_tuple(pt, 1, nullptr));


	std::size_t level = rt1.m_members.leafs_level;
	std::cout << "level: " << level << std::endl;

	//using t_internal_node = typename t_rtree<t_real>::internal_node;
	//const t_internal_node& node = geoidx::detail::rtree::get<t_internal_node>(*rt1.m_members.root);
	//const auto& elements = geoidx::detail::rtree::elements(node);


	Visitor<t_rtree<t_real>> visitor;
	visitor.m_tree = &rt1;

	// see: /usr/include/boost/geometry/index/detail/rtree/node/weak_visitor.hpp
	geoidx::detail::rtree::apply_visitor(visitor, *rt1.m_members.root);


	// box
	t_rtreebox<t_real> globalbounds = rt1.bounds();


	{
		// svg
		std::ofstream ofstr("rtree.svg");
		t_svg<t_real> svg1(ofstr, 100, 100, "width=\"200px\" height=\"200px\"");

		// points
		for(const t_vertex<t_real>& pt : points)
		{
			std::cout << "point: (" << geo::get<0>(pt) << ", " << geo::get<1>(pt) << ")" << std::endl;

			svg1.add(pt);
			svg1.map(pt, "fill; stroke:#000000; stroke-width:1px; fill:#000000;", 2.);
		}

		// bounding boxes
		for(const t_rtreebox<t_real>& bounds : visitor.all_bounds)
		{
			const auto& min = bounds.min_corner();
			const auto& max = bounds.max_corner();

			std::cout << "bounding box min: (" << geo::get<0>(min) << ", " << geo::get<1>(min) << ")";
			std::cout << ", max: (" << geo::get<0>(max) << ", " << geo::get<1>(max) << ")" << std::endl;

			svg1.add(bounds);
			svg1.map(bounds, "stroke:#000000; stroke-width:1px; fill:none; stroke-linecap:round; stroke-linejoin:round;", 1.);
		}

		svg1.add(globalbounds);
		svg1.map(globalbounds, "stroke:#000000; stroke-width:1px; fill:none; stroke-linecap:round; stroke-linejoin:round;", 1.);
	}


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
