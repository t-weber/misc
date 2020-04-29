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
 * g++ -o qhulltst qhulltst.cpp -lqhull_r -lqhullcpp
 */


#include <iostream>
#include <iomanip>
#include <vector>
#include <libqhullcpp/Qhull.h>
#include <libqhullcpp/QhullFacetList.h>
#include <libqhullcpp/QhullVertexSet.h>


namespace qh = orgQhull;
using t_real = coordT;


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

	qh::Qhull qh{"test", dim, int(vec.size()/dim), vec.data(), "Qt"};


	std::cout << "Vertices:" << std::endl;
	qh::QhullVertexList vertices{qh.vertexList()};
	for(auto iter=vertices.begin(); iter!=vertices.end(); ++iter)
	{
		countT id = iter->id();
		qh::QhullPoint pt = iter->point();

		std::cout << "id = " << id << ": ";
		for(std::size_t i=0; i<pt.size(); ++i)
			std::cout << std::setw(10) << pt[i] << " ";
		std::cout << std::endl;
	}


	std::cout << "\nFacets:" << std::endl;
	qh::QhullFacetList facets{qh.facetList()};
	for(auto iterFacet=facets.begin(); iterFacet!=facets.end(); ++iterFacet)
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

		qh::QhullVertexSet vertices = iterFacet->vertices();
		for(auto iterVertex=vertices.begin(); iterVertex!=vertices.end(); ++iterVertex)
		{
			countT id = (*iterVertex).id();
			qh::QhullPoint pt = (*iterVertex).point();

			std::cout << "id = " << id << ": ";
			for(std::size_t i=0; i<pt.size(); ++i)
				std::cout << std::setw(10) << pt[i] << " ";
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}


	std::cout << "Area: " << qh.area() << std::endl;
	std::cout << "Volume: " << qh.volume() << std::endl;

	//std::cout << facets << std::endl;
	return 0;
}