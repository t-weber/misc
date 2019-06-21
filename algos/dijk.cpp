/**
 * shortest path in graph, see e.g. https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
 *
 * @author Tobias Weber
 * @date 15-jun-19
 * @license: see 'LICENSE.EUPL' file
 */

#include <vector>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <string>
#include <iostream>
#include <iomanip>


using t_real = double;
using t_vertex = std::string;
using t_edge = std::tuple<t_vertex, t_vertex, t_real>;
using t_dist = std::tuple<t_real, t_vertex>;	// distance and predecessor


int main()
{
	// edges and distances (weights)
	std::vector<t_edge> edges
	{{
		std::make_tuple("A", "B", 1.),
		std::make_tuple("A", "D", 5.),
		std::make_tuple("B", "C", 10.),
		std::make_tuple("B", "D", 2.),
		std::make_tuple("D", "C", 1.),
		std::make_tuple("C", "A", 5.),
	}};

	// get (unvisited) vertices from the edge endpoints
	std::unordered_set<t_vertex> unvisited, visited;
	for(const auto& edge : edges)
	{
		unvisited.insert(std::get<0>(edge));
		unvisited.insert(std::get<1>(edge));
	}


	// start (or end) vertex
	t_vertex vertcur = "A";
	t_real curdist = 0.;
	std::size_t curiter = 0;
	std::unordered_map<t_vertex, t_dist> distmap;

	while(unvisited.size() != 0)
	{
		std::cout << "Iteration " << ++curiter << std::endl;
		std::cout << "Current vertex: " << vertcur << std::endl;


		// iterate all paths starting from current vertex
		for(const auto& edge : edges)
		{
			const t_vertex* vertfrom = &std::get<0>(edge);
			const t_vertex* vertto = &std::get<1>(edge);
			const t_real dist = std::get<2>(edge);

			// one endpoint of the edge has to be the current vertex
			if(*vertfrom != vertcur && *vertto != vertcur)
				continue;

			// edge has to point from current to new vertex
			if(*vertto == vertcur)
				std::swap(vertfrom, vertto);

			// already seen?
			if(visited.find(*vertto) != visited.end())
				continue;


			auto iter = distmap.find(*vertto);

			if(iter == distmap.end())
			{
				// new entry
				distmap.insert(std::make_pair(*vertto, t_dist{curdist + dist, vertcur}));
				//std::cout << "New entry: " << *vertfrom << " -> " << *vertto << " (" << curdist + dist << ")" << std::endl;
			}
			else
			{
				// update existing entry if the distance is lower
				t_dist& entry = iter->second;
				if(curdist + dist < std::get<0>(entry))
				{
					std::get<0>(entry) = curdist + dist;
					std::get<1>(entry) = vertcur;
				}
			}
		}


		// mark current vertex as visited
		visited.insert(vertcur);
		unvisited.erase(vertcur);


		// find closest unvisited vertex
		t_real closestdist = std::numeric_limits<t_real>::max();
		for(const t_vertex& vert : unvisited)
		{
			auto iter = distmap.find(vert);
			if(iter == distmap.end())
				continue;

			if(std::get<0>(iter->second) < closestdist)
			{
				curdist = closestdist = std::get<0>(iter->second);
				vertcur = vert;
			}
		}


		// output
		std::cout << "Visited: ";
		for(const t_vertex& vert : visited)
			std::cout << vert << " ";
		std::cout << std::endl;

		std::cout
			<< std::setw(15) << " Vertex"
			<< std::setw(15) << " Distance"
			<< std::setw(15) << " Predecessor"
			<< std::endl;
		for(const auto& iter : distmap)
		{
			const t_vertex& vert = iter.first;
			const t_vertex& vertPred = std::get<1>(iter.second);
			const t_real dist = std::get<0>(iter.second);

			std::cout
				<< std::setw(15) << vert
				<< std::setw(15) << dist
				<< std::setw(15) << vertPred
				<< std::endl;
		}
		std::cout << std::endl;
	}

	return 0;
}
