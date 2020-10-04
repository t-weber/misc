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

#include "../libs/math_algos.h"
#include "../libs/math_conts.h"

using t_real = double;
using t_vec = m::vec<t_real, std::vector>;
using t_mat = m::mat<t_real, std::vector>;

using namespace m_ops;


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


std::tuple<const t_vec*, const t_vec*, t_real>
closest_pair_sweep(const std::vector<t_vec>& _points)
{
	std::vector<t_vec> points = _points;
	std::stable_sort(points.begin(), points.end(),
		[](const t_vec& pt1, const t_vec& pt2) -> bool
		{
			return pt1[0] <= pt2[0];
		});


	// TODO
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
		std::cout << "Closest pair (inefficient): point 1: " << *pt1 << ", point 2: " << *pt2
			<< ", dist: " << dist << std::endl;
	}


	{
		auto [pt1, pt2, dist] = closest_pair_sweep(points);
		std::cout << "Closest pair (sweep): point 1: " << *pt1 << ", point 2: " << *pt2
			<< ", dist: " << dist << std::endl;
	}

	return 0;
}
