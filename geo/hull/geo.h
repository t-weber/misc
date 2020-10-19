/**
 * geometric calculations
 * @author Tobias Weber
 * @date 18-Oct-2020
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __GEO2D_H__
#define __GEO2D_H__

#include <vector>
#include <algorithm>

#include "../libs/math_algos.h"
#include "../libs/math_conts.h"

#include <libqhullcpp/Qhull.h>
#include <libqhullcpp/QhullFacet.h>
#include <libqhullcpp/QhullRidge.h>
#include <libqhullcpp/QhullFacetList.h>
#include <libqhullcpp/QhullVertexSet.h>



template<class t_vec>
t_vec calc_circumcentre(const std::vector<t_vec>& triag)
requires m::is_vec<t_vec>
{
	using namespace m_ops;

	using t_real = typename t_vec::value_type;

	if(triag.size() < 3)
		return t_vec{};

	const t_vec& v0 = triag[0];
	const t_vec& v1 = triag[1];
	const t_vec& v2 = triag[2];

	// formula, see: https://de.wikipedia.org/wiki/Umkreis
	const t_real x =
		(v0[0]*v0[0]+v0[1]*v0[1]) * (v1[1]-v2[1]) +
		(v1[0]*v1[0]+v1[1]*v1[1]) * (v2[1]-v0[1]) +
		(v2[0]*v2[0]+v2[1]*v2[1]) * (v0[1]-v1[1]);

	const t_real y =
		(v0[0]*v0[0]+v0[1]*v0[1]) * (v2[0]-v1[0]) +
		(v1[0]*v1[0]+v1[1]*v1[1]) * (v0[0]-v2[0]) +
		(v2[0]*v2[0]+v2[1]*v2[1]) * (v1[0]-v0[0]);

	const t_real n =
		t_real{2}*v0[0] * (v1[1]-v2[1]) +
		t_real{2}*v1[0] * (v2[1]-v0[1]) +
		t_real{2}*v2[0] * (v0[1]-v1[1]);

	return m::create<t_vec>({x/n, y/n});
}



template<class t_vec, class t_real=typename t_vec::value_type>
t_real line_angle(const t_vec& vec1, const t_vec& vec2)
requires m::is_vec<t_vec>
{
	t_vec dir = vec2-vec1;
	return std::atan2(dir[1], dir[0]);
}



template<class t_vec, class t_real=typename t_vec::value_type>
t_real side_of_line(const t_vec& vec1a, const t_vec& vec1b, const t_vec& pt)
requires m::is_vec<t_vec>
{
	//return line_angle<t_vec>(vec1a, pt) - line_angle<t_vec>(vec1a, vec1b);
	using namespace m_ops;

	t_vec dir1 = vec1b - vec1a;
	t_vec dir2 = pt - vec1a;

	return dir1[0]*dir2[1] - dir1[1]*dir2[0];
}



template<class t_vec>
std::vector<t_vec>
_calc_hull_divide_sorted(const std::vector<t_vec>& verts)
requires m::is_vec<t_vec>
{
	using t_real = typename t_vec::value_type;
	t_real eps = 1e-5;

	// trivial cases to end recursion
	if(verts.size() <= 3)
	{
		std::vector<t_vec> hullverts;
		t_vec mean = m::zero<t_vec>(2);
		for(std::size_t vertidx=0; vertidx<verts.size(); ++vertidx)
		{
			mean += verts[vertidx];
			hullverts.push_back(verts[vertidx]);
		}
		mean /= t_real(verts.size());

		std::stable_sort(hullverts.begin(), hullverts.end(), [&mean](const t_vec& vec1, const t_vec& vec2)->bool
		{ return line_angle<t_vec>(mean, vec1) < line_angle<t_vec>(mean, vec2); });

		return hullverts;
	}

	// divide
	std::size_t div = verts.size()/2;
	if(m::equals<t_real>(verts[div-1][0], verts[div][0], eps))
		++div;
	std::vector<t_vec> vertsLeft(verts.begin(), std::next(verts.begin(), div));
	std::vector<t_vec> vertsRight(std::next(verts.begin(), div), verts.end());

	// recurse
	std::vector<t_vec> hullLeft = _calc_hull_divide_sorted(vertsLeft);
	std::vector<t_vec> hullRight = _calc_hull_divide_sorted(vertsRight);

	if(hullLeft.size() == 0) return hullRight;
	if(hullRight.size() == 0) return hullLeft;

	// merge
	// upper part
	{
		auto iterLeftMax = std::max_element(hullLeft.begin(), hullLeft.end(), [](const t_vec& vec1, const t_vec& vec2)->bool
		{ return vec1[0] < vec2[0]; });
		auto iterRightMin = std::min_element(hullRight.rbegin(), hullRight.rend(), [](const t_vec& vec1, const t_vec& vec2)->bool
		{ return vec1[0] < vec2[0]; });

		std::rotate(hullLeft.begin(), iterLeftMax, hullLeft.end());
		std::rotate(hullRight.rbegin(), iterRightMin, hullRight.rend());

		auto iterLeft = hullLeft.begin();
		auto iterRight = hullRight.rbegin();
		bool leftFound=false, rightFound=false;
		while(true)
		{
			if(!leftFound)
			{
				auto iterLeftNext = std::next(iterLeft, 1);
				if(iterLeftNext == hullLeft.end())
					leftFound = true;
				else
				{
					if(side_of_line(*iterLeft, *iterRight, *iterLeftNext) < 0.)
						leftFound = true;
					else
						iterLeft = iterLeftNext;
				}
			}

			if(!rightFound)
			{
				auto iterRightNext = std::next(iterRight, 1);
				if(iterRightNext == hullRight.rend())
					rightFound = true;
				else
				{
					if(side_of_line(*iterLeft, *iterRight, *iterRightNext) < 0.)
						rightFound = true;
					else
						iterRight = iterRightNext;
				}
			}

			if(leftFound && rightFound)
				break;
		}

		auto iterLeftStart = std::next(hullLeft.begin(), 1);
		auto iterLeftEnd = iterLeft;
		if(iterLeftStart < iterLeftEnd)
			hullLeft.erase(iterLeftStart, iterLeftEnd);

		auto iterRightStart = std::next(std::next(iterRight,1).base());
		auto iterRightEnd = std::prev(hullRight.end(),1);
		if(iterRightStart < iterRightEnd)
			hullRight.erase(iterRightStart, iterRightEnd);
	}

	if(hullLeft.size() == 0) return hullRight;
	if(hullRight.size() == 0) return hullLeft;

	// lower part
	{
		auto iterLeftMax = std::max_element(hullLeft.rbegin(), hullLeft.rend(), [](const t_vec& vec1, const t_vec& vec2)->bool
		{ return vec1[0] < vec2[0]; });
		auto iterRightMin = std::min_element(hullRight.begin(), hullRight.end(), [](const t_vec& vec1, const t_vec& vec2)->bool
		{ return vec1[0] < vec2[0]; });

		std::rotate(hullLeft.rbegin(), iterLeftMax, hullLeft.rend());
		std::rotate(hullRight.begin(), iterRightMin, hullRight.end());
		//std::rotate(hullLeft.begin(), std::next(hullLeft.begin(),1), hullLeft.end());
		//std::rotate(hullRight.begin(), std::prev(hullRight.end(),1), hullRight.end());

		auto iterLeft = hullLeft.rbegin();
		auto iterRight = hullRight.begin();
		bool leftFound=false, rightFound=false;
		while(true)
		{
			if(!leftFound)
			{
				auto iterLeftNext = std::next(iterLeft, 1);
				if(iterLeftNext == hullLeft.rend())
					leftFound = true;
				else
				{
					if(side_of_line(*iterLeft, *iterRight, *iterLeftNext) > 0.)
						leftFound = true;
					else
						iterLeft = iterLeftNext;
				}
			}

			if(!rightFound)
			{
				auto iterRightNext = std::next(iterRight, 1);
				if(iterRightNext == hullRight.end())
					rightFound = true;
				else
				{
					if(side_of_line(*iterLeft, *iterRight, *iterRightNext) > 0.)
						rightFound = true;
					else
						iterRight = iterRightNext;
				}
			}

			if(leftFound && rightFound)
				break;
		}

		auto iterLeftStart = std::next(std::next(iterLeft,1).base(), 1);
		auto iterLeftEnd = hullLeft.end();
		if(iterLeftStart < iterLeftEnd)
			hullLeft.erase(iterLeftStart, iterLeftEnd);

		auto iterRightStart = hullRight.begin();
		auto iterRightEnd = iterRight;
		if(iterRightStart < iterRightEnd)
			hullRight.erase(iterRightStart, iterRightEnd);
	}

	hullLeft.insert(hullLeft.end(), hullRight.begin(), hullRight.end());

	t_vec mean = std::accumulate(hullLeft.begin(), hullLeft.end(), m::zero<t_vec>(2));
	mean /= t_real(hullLeft.size());
	std::stable_sort(hullLeft.begin(), hullLeft.end(), [&mean](const t_vec& vec1, const t_vec& vec2)->bool
	{ return line_angle<t_vec>(mean, vec1) < line_angle<t_vec>(mean, vec2); });

	return hullLeft;
}



template<class t_vec>
std::vector<t_vec>
calc_hull_divide(const std::vector<t_vec>& _verts)
requires m::is_vec<t_vec>
{
	using t_real = typename t_vec::value_type;
	t_real eps = 1e-5;
	std::vector<t_vec> verts = _verts;

	// sort
	std::stable_sort(verts.begin(), verts.end(), [eps](const t_vec& vert1, const t_vec& vert2) -> bool
	{
		if(m::equals<t_real>(vert1[0], vert2[0], eps))
			return vert1[1] < vert2[1];
		return vert1[0] < vert2[0];
	});

	// remove unnecessary points
	auto iterCurX = verts.begin();
	for(auto iter = iterCurX; iter != verts.end(); std::advance(iter, 1))
	{
		// next x?
		if(!m::equals<t_real>((*iter)[0], (*iterCurX)[0], eps))
		{
			std::size_t num_same_x = iter - iterCurX;
			if(num_same_x >= 3)
				iter = verts.erase(std::next(iterCurX, 1), iter);
			iterCurX = iter;
		}
	}

	return _calc_hull_divide_sorted<t_vec>(verts);
}



/**
 * delaunay triangulation and voronoi vertices
 */
template<class t_vec>
std::tuple<std::vector<t_vec>, std::vector<std::vector<t_vec>>>
calc_delaunay(int dim, const std::vector<t_vec>& verts, bool only_hull)
requires m::is_vec<t_vec>
{
	using namespace m_ops;
	namespace qh = orgQhull;

	using t_real = typename t_vec::value_type;
	using t_real_qhull = coordT;

	std::vector<t_vec> voronoi;				// voronoi vertices
	std::vector<std::vector<t_vec>> triags;	// delaunay triangles

	try
	{
		std::vector<t_real_qhull> _verts;
		_verts.reserve(verts.size() * dim);
		for(const t_vec& vert : verts)
			for(int i=0; i<dim; ++i)
				_verts.push_back(t_real_qhull{vert[i]});

		qh::Qhull qh{"triag", dim, int(_verts.size()/dim), _verts.data(), only_hull ? "Qt" : "v Qu QJ" };
		if(qh.hasQhullMessage())
			std::cout << qh.qhullMessage() << std::endl;


		//qh::QhullVertexList vertices{qh.vertexList()};
		qh::QhullFacetList facets{qh.facetList()};

		for(auto iterFacet=facets.begin(); iterFacet!=facets.end(); ++iterFacet)
		{
			if(iterFacet->isUpperDelaunay())
				continue;

			if(!only_hull)
			{
				qh::QhullPoint pt = iterFacet->voronoiVertex();

				t_vec vec = m::create<t_vec>(dim);
				for(int i=0; i<dim; ++i)
					vec[i] = t_real{pt[i]};

				voronoi.emplace_back(std::move(vec));
			}


			std::vector<t_vec> thetriag;
			qh::QhullVertexSet vertices = iterFacet->vertices();

			for(auto iterVertex=vertices.begin(); iterVertex!=vertices.end(); ++iterVertex)
			{
				qh::QhullPoint pt = (*iterVertex).point();

				t_vec vec = m::create<t_vec>(dim);
				for(int i=0; i<dim; ++i)
					vec[i] = t_real{pt[i]};

				thetriag.emplace_back(std::move(vec));
			}

			triags.emplace_back(std::move(thetriag));
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return std::make_tuple(voronoi, triags);
}



/**
 * delaunay triangulation using parabolic trafo
 */
template<class t_vec>
std::tuple<std::vector<t_vec>, std::vector<std::vector<t_vec>>>
calc_delaunay_parabolic(const std::vector<t_vec>& verts)
requires m::is_vec<t_vec>
{
	using namespace m_ops;
	namespace qh = orgQhull;

	using t_real = typename t_vec::value_type;
	using t_real_qhull = coordT;

	const int dim = 2;
	std::vector<t_vec> voronoi;				// voronoi vertices
	std::vector<std::vector<t_vec>> triags;	// delaunay triangles

	try
	{
		std::vector<t_real_qhull> _verts;
		_verts.reserve(verts.size()*(dim+1));
		for(const t_vec& vert : verts)
		{
			_verts.push_back(t_real_qhull{vert[0]});
			_verts.push_back(t_real_qhull{vert[1]});
			_verts.push_back(t_real_qhull{vert[0]*vert[0] + vert[1]*vert[1]});
		}

		qh::Qhull qh{"triag", dim+1, int(_verts.size()/(dim+1)), _verts.data(), "Qt"};
		if(qh.hasQhullMessage())
			std::cout << qh.qhullMessage() << std::endl;


		qh::QhullFacetList facets{qh.facetList()};

		for(auto iterFacet=facets.begin(); iterFacet!=facets.end(); ++iterFacet)
		{
			if(iterFacet->isUpperDelaunay())
				continue;

			// filter out non-visible part of hull
			qh::QhullHyperplane plane = iterFacet->hyperplane();
			t_vec normal = m::create<t_vec>(dim+1);
			for(int i=0; i<dim+1; ++i)
				normal[i] = t_real{plane[i]};
			// normal pointing upwards?
			if(normal[2] > 0.)
				continue;

			std::vector<t_vec> thetriag;
			qh::QhullVertexSet vertices = iterFacet->vertices();

			for(auto iterVertex=vertices.begin(); iterVertex!=vertices.end(); ++iterVertex)
			{
				qh::QhullPoint pt = (*iterVertex).point();

				t_vec vec = m::create<t_vec>(dim);
				for(int i=0; i<dim; ++i)
					vec[i] = t_real{pt[i]};

				thetriag.emplace_back(std::move(vec));
			}

			voronoi.emplace_back(calc_circumcentre<t_vec>(thetriag));
			triags.emplace_back(std::move(thetriag));
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return std::make_tuple(voronoi, triags);
}


#endif
