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
#include <queue>
#include <algorithm>

#include <boost/intrusive/avltree.hpp>
namespace intr = boost::intrusive;

#include "libs/math_algos.h"
#include "libs/math_conts.h"
using namespace m_ops;


using t_real = double;
using t_vec = m::vec<t_real, std::vector>;
using t_mat = m::mat<t_real, std::vector>;

using t_line = std::pair<t_vec, t_vec>;
using t_lines = std::vector<t_line>;


static const t_real g_eps = 1e-6;


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


t_real get_line_y(const t_line& line, t_real x)
{
	const t_vec& pt1 = std::get<0>(line);
	const t_vec& pt2 = std::get<1>(line);

	t_real slope = (pt2[1]-pt1[1]) / (pt2[0]-pt1[0]);

	return pt1[1] + (x-pt1[0])*slope;
}



std::vector<std::tuple<std::size_t, std::size_t, t_vec>>
intersect_ineff(const t_lines& lines)
{
	std::vector<std::tuple<std::size_t, std::size_t, t_vec>> intersections;

	for(std::size_t i=0; i<lines.size(); ++i)
	{
		for(std::size_t j=i+1; j<lines.size(); ++j)
		{
			const t_line& line1 = lines[i];
			const t_line& line2 = lines[j];

			if(auto [intersects, pt] = intersect_lines(line1, line2); intersects)
				intersections.emplace_back(std::make_tuple(i, j, pt));
		}
	}

	return intersections;
}



template<class t_hook>
struct TreeLeaf
{
	const t_real *curX{nullptr};
	const t_lines *lines{nullptr};
	std::size_t line_idx{0};

	t_hook _h{};


	friend std::ostream& operator<<(std::ostream& ostr, const TreeLeaf<t_hook>& e)
	{
		ostr << std::get<0>((*e.lines)[e.line_idx]) << ", " << std::get<1>((*e.lines)[e.line_idx]);
		return ostr;
	}

	friend bool operator<(const TreeLeaf<t_hook>& e1, const TreeLeaf<t_hook>& e2)
	{
		t_real line1_y = get_line_y((*e1.lines)[e1.line_idx], *e1.curX);
		t_real line2_y = get_line_y((*e2.lines)[e2.line_idx], *e2.curX);

		// compare by y
		return line1_y < line2_y;
	}
};

using t_leaf = TreeLeaf<intr::avl_set_member_hook<intr::link_mode<intr::normal_link>>>;
using t_tree = intr::avltree<t_leaf, intr::member_hook<t_leaf, decltype(t_leaf::_h), &t_leaf::_h>>;



enum class SweepEventType { LEFT_VERTEX, RIGHT_VERTEX, INTERSECTION };

struct SweepEvent
{
	t_real x;
	SweepEventType ty{SweepEventType::LEFT_VERTEX};

	std::size_t line_idx{};
	std::optional<std::size_t> lower_idx{}, upper_idx{};
	std::optional<t_vec> intersection{};


	friend std::ostream& operator<<(std::ostream& ostr, const SweepEvent& evt)
	{
		std::string strty;
		if(evt.ty == SweepEventType::LEFT_VERTEX)
			strty = "left_vertex";
		else if(evt.ty == SweepEventType::RIGHT_VERTEX)
			strty = "right_vertex";
		else if(evt.ty == SweepEventType::INTERSECTION)
			strty = "intersection";

		ostr << "x=" << std::setw(6) << evt.x << ", type=" << std::setw(12)
			<< strty << ", line " << evt.line_idx;
		if(evt.lower_idx)
			ostr << ", lower=" << *evt.lower_idx;
		if(evt.upper_idx)
			ostr << ", upper=" << *evt.upper_idx;

		if(evt.intersection)
			ostr << ", intersection=" << *evt.intersection;

		return ostr;
	}
};



std::vector<std::tuple<std::size_t, std::size_t, t_vec>>
intersect_sweep(const t_lines& lines)
{
	// events
	auto events_comp = [](const SweepEvent& evt1, const SweepEvent& evt2) -> bool { return evt1.x > evt2.x; };
	std::priority_queue<SweepEvent, std::vector<SweepEvent>, decltype(events_comp)> events(events_comp);
	for(std::size_t line_idx=0; line_idx<lines.size(); ++line_idx)
	{
		const t_line& line = lines[line_idx];

		SweepEvent evtLeft{.x = std::get<0>(line)[0], .ty=SweepEventType::LEFT_VERTEX, .line_idx=line_idx};
		events.emplace(std::move(evtLeft));

		SweepEvent evtRight{.x = std::get<1>(line)[0], .ty=SweepEventType::RIGHT_VERTEX, .line_idx=line_idx};
		events.emplace(std::move(evtRight));
	}


	// status
	t_tree status;

	// results
	std::vector<std::tuple<std::size_t, std::size_t, t_vec>> intersections;

	t_real curX = 0.;
	while(events.size())
	{
		SweepEvent evt{std::move(events.top())};
		events.pop();

		curX = evt.x;
		//std::cout << "Event: " << evt << "." << std::endl;

		switch(evt.ty)
		{
			case SweepEventType::LEFT_VERTEX:
			{
				// activate line
				t_leaf *leaf = new t_leaf{.curX=&curX, .lines=&lines, .line_idx=evt.line_idx};
				auto iter = status.insert_equal(*leaf);

				auto iterPrev = std::prev(iter, 1);
				auto iterNext = std::next(iter, 1);


				// add possible intersection event
				if(iterPrev != iter && iterPrev != status.end())
				{
					//std::cout << "index: " << evt.line_idx << ", prev: " << iterPrev->line_idx << std::endl;

					const t_line& line = lines[evt.line_idx];
					const t_line& linePrev = lines[iterPrev->line_idx];

					if(auto [intersects, pt] = intersect_lines(line, linePrev);
						intersects && !m::equals<t_real>(curX, pt[0], g_eps))
					{
						SweepEvent evtPrev{.x = pt[0], .ty=SweepEventType::INTERSECTION,
							.lower_idx=iterPrev->line_idx, .upper_idx=evt.line_idx,
							.intersection=pt};
						events.emplace(std::move(evtPrev));
					}
				}

				// add possible intersection event
				if(iterNext != iter && iterNext != status.end())
				{
					//std::cout << "index: " << evt.line_idx << ", next: " << iterNext->line_idx << std::endl;

					const t_line& line = lines[evt.line_idx];
					const t_line& lineNext = lines[iterNext->line_idx];

					if(auto [intersects, pt] = intersect_lines(line, lineNext);
						intersects  && !m::equals<t_real>(curX, pt[0], g_eps))
					{
						SweepEvent evtNext{.x = pt[0], .ty=SweepEventType::INTERSECTION,
							.lower_idx=evt.line_idx, .upper_idx=iterNext->line_idx,
							.intersection=pt};
						events.emplace(std::move(evtNext));
					}
				}

				break;
			}
			case SweepEventType::RIGHT_VERTEX:
			{
				// find current line
				auto iter = std::find_if(status.begin(), status.end(), [&evt](const auto& leaf) -> bool
				{ return leaf.line_idx == evt.line_idx; });

				if(iter == status.end())
					continue;

				auto iterPrev = std::prev(iter, 1);
				auto iterNext = std::next(iter, 1);

				// inactivate current line
				delete &*iter;
				iter = status.erase(iter);


				// add possible intersection event
				if(iterPrev != iterNext && iterPrev != status.end() && iterNext != status.end())
				{
					//std::cout << "prev: " << iterPrev->line_idx << ", next: " << iterNext->line_idx << std::endl;

					const t_line& linePrev = lines[iterPrev->line_idx];
					const t_line& lineNext = lines[iterNext->line_idx];

					if(auto [intersects, pt] = intersect_lines(linePrev, lineNext);
						intersects && !m::equals<t_real>(curX, pt[0], g_eps))
					{
						SweepEvent evt{.x = pt[0], .ty=SweepEventType::INTERSECTION,
							.lower_idx=iterPrev->line_idx, .upper_idx=iterNext->line_idx,
							.intersection=pt};
							events.emplace(std::move(evt));
					}
				}

				break;
			}
			case SweepEventType::INTERSECTION:
			{
				if(std::find_if(intersections.begin(), intersections.end(), [&evt](const auto& inters) -> bool
					{ return m::equals<t_vec>(std::get<2>(inters), *evt.intersection, g_eps); }) == intersections.end())
				{
					intersections.emplace_back(std::make_tuple(*evt.lower_idx, *evt.upper_idx, *evt.intersection));
				}

				// find upper line
				auto iterUpper = std::find_if(status.begin(), status.end(), [&evt](const auto& leaf) -> bool
				{ return leaf.line_idx == evt.upper_idx; });

				// find lower line
				auto iterLower = std::find_if(status.begin(), status.end(), [&evt](const auto& leaf) -> bool
				{ return leaf.line_idx == evt.lower_idx; });


				std::swap(iterUpper->line_idx, iterLower->line_idx);
				std::swap(iterUpper, iterLower);


				auto iterPrev = std::prev(iterUpper, 1);
				auto iterNext = std::next(iterLower, 1);


				// add possible intersection event
				if(iterPrev != iterUpper && iterPrev != status.end() && iterUpper != status.end())
				{
					//std::cout << "prev: " << iterPrev->line_idx << ", upper: " << iterUpper->line_idx << std::endl;

					const t_line& linePrev = lines[iterPrev->line_idx];
					const t_line& lineUpper = lines[iterUpper->line_idx];

					if(auto [intersects, pt] = intersect_lines(linePrev, lineUpper);
						intersects && !m::equals<t_real>(curX, pt[0], g_eps))
					{
						SweepEvent evtPrev{.x = pt[0], .ty=SweepEventType::INTERSECTION,
							.lower_idx=iterPrev->line_idx, .upper_idx=iterUpper->line_idx,
							.intersection=pt};
						events.emplace(std::move(evtPrev));
					}
				}

				// add possible intersection event
				if(iterNext != iterLower && iterNext != status.end() && iterLower != status.end())
				{
					//std::cout << "next: " << iterNext->line_idx << ", lower: " << iterLower->line_idx << std::endl;

					const t_line& lineNext = lines[iterNext->line_idx];
					const t_line& lineLower = lines[iterLower->line_idx];

					if(auto [intersects, pt] = intersect_lines(lineNext, lineLower);
						intersects  && !m::equals<t_real>(curX, pt[0], g_eps))
					{
						SweepEvent evtNext{.x = pt[0], .ty=SweepEventType::INTERSECTION,
							.lower_idx=iterLower->line_idx, .upper_idx=iterNext->line_idx,
							.intersection=pt};
						events.emplace(std::move(evtNext));
					}
				}

				break;
			}
		}
	}

	return intersections;
}



int main()
{
	t_lines lines{{
		std::make_pair(m::create<t_vec>({1., 2.}), m::create<t_vec>({2., 2.})),
		std::make_pair(m::create<t_vec>({1.9, 1.}), m::create<t_vec>({2.1, 3.})),
		std::make_pair(m::create<t_vec>({1.8, 1.1}), m::create<t_vec>({2.2, 3.1})),
		std::make_pair(m::create<t_vec>({0., 0.}), m::create<t_vec>({6., 5.})),
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

