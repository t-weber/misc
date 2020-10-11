/**
 * line segment intersections
 * @author Tobias Weber
 * @date 11-oct-20
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- http://dx.doi.org/10.1007/3-540-27619-X, ch 2.3.2, p. 64
 */

#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

#include <boost/intrusive/avltree.hpp>
namespace intr = boost::intrusive;

#include "../libs/math_algos.h"
#include "../libs/math_conts.h"
using namespace m_ops;


using t_real = double;
using t_vec = m::vec<t_real, std::vector>;
using t_mat = m::mat<t_real, std::vector>;

using t_line = std::pair<t_vec, t_vec>;


template<class t_hook, class T>
struct TreeLeaf
{
	const T* vec = nullptr;
	t_hook _h{};

	friend std::ostream& operator<<(std::ostream& ostr, const TreeLeaf<t_hook, T>& e)
	{
		ostr << *e.vec;
		return ostr;
	}

	friend bool operator<(const TreeLeaf<t_hook, T>& e1, const TreeLeaf<t_hook, T>& e2)
	{
		// compare by y
		return (*e1.vec)[1] < (*e2.vec)[1];
	}
};

using t_leaf = TreeLeaf<intr::avl_set_member_hook<intr::link_mode<intr::normal_link>>, t_vec>;
using t_tree = intr::avltree<t_leaf, intr::member_hook<t_leaf, decltype(t_leaf::_h), &t_leaf::_h>>;



std::tuple<bool, t_vec>
intersect_lines(const t_line& line1, const t_line& line2)
{
	const t_vec& pos1 = std::get<0>(line1);
	const t_vec& pos2 = std::get<0>(line2);
	t_vec dir1 = std::get<1>(line1) - pos1;
	t_vec dir2 = std::get<1>(line2) - pos2;

	auto[pt1, pt2, valid, dist, param1, param2] =
		m::intersect_line_line(pos1, dir1, pos2, dir2);
	if(!valid || param1<0. || param1>1. || param2<0. || param2>1.)
		return std::make_pair(false, m::create<t_vec>({}));

	return std::make_pair(true, pt1);
}



std::vector<std::tuple<std::size_t, std::size_t, t_vec>>
intersect_ineff(const std::vector<t_line>& lines)
{
	std::vector<std::tuple<std::size_t, std::size_t, t_vec>> intersections;

	for(std::size_t i=0; i<lines.size(); ++i)
	{
		for(std::size_t j=i+1; j<lines.size(); ++j)
		{
			const t_line& line1 = lines[i];
			const t_line& line2 = lines[j];

			auto [intersects, pt] = intersect_lines(line1, line2);
			if(intersects)
				intersections.emplace_back(std::make_tuple(i, j, pt));
		}
	}

	return intersections;
}



std::vector<std::tuple<std::size_t, std::size_t, t_vec>>
intersect_sweep(const std::vector<t_line>& lines)
{
	std::vector<std::tuple<std::size_t, std::size_t, t_vec>> intersections;

	// TODO

	return intersections;
}



int main()
{
	std::vector<t_line> lines{{
		std::make_pair(m::create<t_vec>({1., 2.}), m::create<t_vec>({3., 2.})),
		std::make_pair(m::create<t_vec>({2., 1.}), m::create<t_vec>({2., 3.})),
	}};


	{
		auto intersections = intersect_ineff(lines);
		for(const auto& intersection : intersections)
		{
			std::cout << "Intersection between line " << std::get<0>(intersection)
				<< " and line " << std::get<1>(intersection)
				<< ": " << std::get<2>(intersection) 
				<< "." << std::endl;
		}
	}

	std::cout << std::endl;

	{
		auto intersections = intersect_sweep(lines);
		for(const auto& intersection : intersections)
		{
			std::cout << "Intersection between line " << std::get<0>(intersection)
				<< " and line " << std::get<1>(intersection)
				<< ": " << std::get<2>(intersection) 
				<< "." << std::endl;
		}
	}

	return 0;	
}

