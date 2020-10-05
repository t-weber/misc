/**
 * closest pair
 * @author Tobias Weber
 * @date 4-oct-20
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- http://dx.doi.org/10.1007/3-540-27619-X, ch 2.3.1, p. 57
 *	- https://en.wikipedia.org/wiki/Closest_pair_of_points_problem
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



std::tuple<const t_vec*, const t_vec*, t_real>
closest_pair_ineff(const std::vector<t_vec>& points)
{
	const t_vec* pt1 = nullptr;
	const t_vec* pt2 = nullptr;
	t_real dist = std::numeric_limits<t_real>::max();

	for(std::size_t i=0; i<points.size(); ++i)
	{
		for(std::size_t j=i+1; j<points.size(); ++j)
		{
			t_real norm = m::norm<t_vec>(points[i] - points[j]);
			if(norm < dist)
			{
				dist = norm;
				pt1 = &points[i];
				pt2 = &points[j];
			}
		}
	}

	return std::make_tuple(pt1, pt2, dist);
}


std::tuple<t_vec, t_vec, t_real>
closest_pair_sweep(const std::vector<t_vec>& points)
{
	std::vector<t_leaf> leaves;
	for(const t_vec& pt : points)
		leaves.emplace_back(t_leaf{.vec = &pt});

	std::stable_sort(leaves.begin(), leaves.end(),
		[](const t_leaf& leaf1, const t_leaf& leaf2) -> bool
		{
			// sort by x
			return (*leaf1.vec)[0] <= (*leaf2.vec)[0];
		});


	t_leaf* leaf1 = &leaves[0];
	t_leaf* leaf2 = &leaves[1];
	t_real dist = m::norm<t_vec>(*leaf1->vec - *leaf2->vec);

	t_tree status;
	status.insert_equal(*leaf1);
	status.insert_equal(*leaf2);


	auto iter1 = leaves.begin();
	for(auto iter2 = std::next(leaves.begin(), 2); iter2 != leaves.end(); )
	{
		if((*iter1->vec)[0] <= (*iter2->vec)[0]-dist)
		{
			// remove dead elements
			//std::cout << "Removing " << *iter1->vec << std::endl;
			status.erase(*iter1);
			std::advance(iter1, 1);
		}
		else
		{
			// insert newly active elements
			t_vec vec1 = *iter2->vec; vec1[1] -= dist;
			t_vec vec2 = *iter2->vec; vec2[1] += dist;
			auto [iterrange1, iterrange2] =
				status.bounded_range(t_leaf{.vec=&vec1}, t_leaf{.vec=&vec2}, 1, 1);

			for(auto iter=iterrange1; iter!=iterrange2; std::advance(iter,1))
			{
				t_real newdist = m::norm<t_vec>(*iter->vec - *iter2->vec);
				if(newdist < dist)
				{
					dist = newdist;
					leaf1 = &*iter;
					leaf2 = &*iter2;
				}
			}

			//std::cout << "Inserting " << *iter2->vec << std::endl;
			status.insert_equal(*iter2);
			std::advance(iter2, 1);
		}
	}

	return std::make_tuple(*leaf1->vec, *leaf2->vec, dist);
}


int main()
{
	std::vector<t_vec> points{{
		m::create<t_vec>({1., 0.}),
		m::create<t_vec>({2., 0.5}),
		m::create<t_vec>({3., 7.}),
		m::create<t_vec>({4., 4.}),
		m::create<t_vec>({5., 2.}),
		m::create<t_vec>({6., 3.}),
		m::create<t_vec>({7., 1.}),
		m::create<t_vec>({8., 5.}),
		m::create<t_vec>({9., 5.}),
	}};

	{
		auto [pt1, pt2, dist] = closest_pair_ineff(points);
		if(pt1 && pt2)
		{
			std::cout << "Closest pair (inefficient): point 1: " << *pt1 << ", point 2: " << *pt2
				<< ", dist: " << dist << std::endl;
		}
	}


	{
		auto [pt1, pt2, dist] = closest_pair_sweep(points);
		std::cout << "Closest pair (sweep): point 1: " << pt1 << ", point 2: " << pt2
			<< ", dist: " << dist << std::endl;
	}

	return 0;
}
