/**
 * testing qhull
 * @author Tobias Weber
 * @date 24-apr-20
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- http://www.qhull.org/html/qh-code.htm#cpp
 *	- https://github.com/qhull/qhull/tree/master/src/libqhullcpp
 *	- https://github.com/qhull/qhull/blob/master/src/qhulltest/Qhull_test.cpp
 *
 * g++ -std=c++20 -o qhulltst qhulltst.cpp -lqhull_r -lqhullcpp
 */


#include <iostream>
#include <iomanip>
#include <vector>
#include <ranges>
#include <libqhullcpp/Qhull.h>
#include <libqhullcpp/QhullFacet.h>
#include <libqhullcpp/QhullFacetList.h>
#include <libqhullcpp/QhullVertexSet.h>


namespace qh = orgQhull;
using t_real = coordT;


template<class t_vertices>
static void print_vertices(const t_vertices& vertices)
{
	for(auto iterVertex=vertices.begin(); iterVertex!=vertices.end(); ++iterVertex)
	{
		countT id = (*iterVertex).id();
		qh::QhullPoint pt = (*iterVertex).point();

		std::cout << "id = " << std::setw(5) << id << ": ";
		for(std::size_t i=0; i<pt.size(); ++i)
			std::cout << std::setw(10) << pt[i] << " ";
		std::cout << std::endl;
	}
}


template<class t_facetiter>
static void print_facetinfo(const t_facetiter& iterFacet)
{
	std::cout << "good: " << std::boolalpha << iterFacet->isGood() << ", ";
	std::cout << "top orient: " << std::boolalpha << iterFacet->isTopOrient() << ", ";
	std::cout << "simplicial: " << std::boolalpha << iterFacet->isSimplicial() << ", ";
	std::cout << "upper delaunay: " << std::boolalpha << iterFacet->isUpperDelaunay() << std::endl;

	// get plane equation
	const facetT* facett = iterFacet->getFacetT();
	if(facett)
	{
		const t_real* normal = facett->normal;
		std::cout << "plane normal: " << normal[0] << " " << normal[1] << " " << normal[2] << std::endl;
		std::cout << "plane offset: " << facett->offset  << std::endl;
	}
}


static void hull(int dim, const std::vector<t_real>& vec)
{
	qh::Qhull qh{"test", dim, int(vec.size()/dim), std::ranges::data(vec), "Qt" /*see: man qhull*/ };

	std::cout << "Vertices:" << std::endl;
	qh::QhullVertexList vertices{qh.vertexList()};
	print_vertices(vertices);

	std::cout << "\nFacets:" << std::endl;
	qh::QhullFacetList facets{qh.facetList()};
	for(auto iterFacet=facets.begin(); iterFacet!=facets.end(); ++iterFacet)
	{
		print_facetinfo(iterFacet);

		qh::QhullVertexSet vertices = iterFacet->vertices();
		print_vertices(vertices);
		std::cout << std::endl;
	}


	std::cout << "Area: " << qh.area() << std::endl;
	std::cout << "Volume: " << qh.volume() << std::endl;

	//std::cout << facets << std::endl;
}


static void voronoi(int dim, const std::vector<t_real>& vec)
{
	qh::Qhull qh{"test", dim, int(vec.size()/dim), std::ranges::data(vec), "v" /*see: man qhull*/ };

	std::cout << "Vertices:" << std::endl;
	qh::QhullVertexList vertices{qh.vertexList()};
	print_vertices(vertices);

	std::cout << "\nFacets:" << std::endl;
	qh::QhullFacetList facets{qh.facetList()};
	for(auto iterFacet=facets.begin(); iterFacet!=facets.end(); ++iterFacet)
	{
		print_facetinfo(iterFacet);

		qh::QhullPoint vorvert = iterFacet->voronoiVertex();
		std::cout << "voronoi vertex: " << vorvert << std::endl;
	}
}



int main()
{
	const int dim = 3;
	std::vector<t_real> vec
	{{
		-10.,   0.,   0.,
		 10.,   0.,   0.,
		  0.,  10.,   0.,
		  0., -10.,   0.,
		  0.,   0., -10.,
		  0.,   0.,  10.,

		 -5.,   0.,   0.,
		  5.,   0.,   0.,
		  0.,   5.,   0.,
		  0.,  -5.,   0.,
		  0.,   0.,  -5.,
		  0.,   0.,   5.,
	}};

	hull(dim, vec);
	std::cout << "\n--------------------------------------------------------------------------------\n" << std::endl;;

	voronoi(dim, vec);
	std::cout << "\n--------------------------------------------------------------------------------\n" << std::endl;;

	return 0;
}
