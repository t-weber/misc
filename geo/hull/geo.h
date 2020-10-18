/**
 * geometric calculations
 * @author Tobias Weber
 * @date 18-Oct-2020
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __GEO2D_H__
#define __GEO2D_H__

#include <vector>

#include "../libs/math_algos.h"
#include "../libs/math_conts.h"

#include <libqhullcpp/Qhull.h>
#include <libqhullcpp/QhullFacet.h>
#include <libqhullcpp/QhullRidge.h>
#include <libqhullcpp/QhullFacetList.h>
#include <libqhullcpp/QhullVertexSet.h>



template<class t_vec>
t_vec calc_circumcentre(const std::vector<t_vec>& triag)
requires m::is_basic_vec<t_vec>
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



template<class t_vec>
std::vector<std::vector<t_vec>>
calc_hull_divide(const std::vector<t_vec>& verts)
requires m::is_basic_vec<t_vec>
{
	std::vector<std::vector<t_vec>> hull;

	// TODO

	return hull;
}



/**
 * delaunay triangulation and voronoi vertices
 */
template<class t_vec>
std::tuple<std::vector<t_vec>, std::vector<std::vector<t_vec>>>
calc_delaunay(int dim, const std::vector<t_vec>& verts, bool only_hull)
requires m::is_basic_vec<t_vec>
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
requires m::is_basic_vec<t_vec>
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
