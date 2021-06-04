/**
 * graph algorithms
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date may-2021
 * @license see 'LICENSE.EUPL' file
 *
 * references:
 *   - (FUH 2021) "Effiziente Algorithmen" (2021), Kurs 1684, Fernuni Hagen
 *                (https://vu.fernuni-hagen.de/lvuweb/lvu/app/Kurs/01684).
 */

#ifndef __GRAPH_ALGOS_H__
#define __GRAPH_ALGOS_H__

#include <vector>
#include <queue>
#include <limits>
#include <concepts>
#include <iostream>

#include "math_conts.h"
#include "math_algos.h"


// ----------------------------------------------------------------------------
// concept for the container interface
// ----------------------------------------------------------------------------
template<class t_graph>
concept is_graph = requires(t_graph& graph, std::size_t vertidx, typename t_graph::t_weight w)
{
	{ graph.GetNumVertices() } -> std::convertible_to<std::size_t>;
	{ graph.GetVertexIdent(vertidx) } -> std::convertible_to<std::string>;

	graph.SetWeight(vertidx, vertidx, w);
	{ graph.GetWeight(vertidx, vertidx) } -> std::convertible_to<typename t_graph::t_weight>;

	graph.GetNeighbours(vertidx);

	graph.AddVertex("123");
	graph.AddEdge(0, 1);
	graph.AddEdge("1", "2");
};


template<class t_graph>
concept is_flux_graph = requires(t_graph& graph, std::size_t vertidx, typename t_graph::t_weight w)
{
	requires is_graph<t_graph>;
	requires is_pair<typename t_graph::t_data>;

	graph.SetCapacity(vertidx, vertidx, w);
	{ graph.GetCapacity(vertidx, vertidx) } -> std::convertible_to<typename t_graph::t_weight>;
};
// ----------------------------------------------------------------------------


/**
 * export graph to dot
 * @see https://graphviz.org/doc/info/lang.html
 */
template<class t_graph> requires (is_graph<t_graph> && !is_flux_graph<t_graph>)
void print_graph(const t_graph& graph, std::ostream& ostr = std::cout)
{
	const std::size_t N = graph.GetNumVertices();

	ostr << "digraph my_graph\n{\n";

	ostr << "\t// vertices\n";
	for(std::size_t i=0; i<N; ++i)
		ostr << "\t" << i << " [label=\"" << graph.GetVertexIdent(i) << "\"];\n";

	ostr << "\n";
	ostr << "\t// edges and weights\n";

	for(std::size_t i=0; i<N; ++i)
	{
		for(std::size_t j=0; j<N; ++j)
		{
			typename t_graph::t_weight w = graph.GetWeight(i, j);
			if(!w)
				continue;

			ostr << "\t" << i << " -> " << j << " [label=\"" << w << "\"];\n";
		}
	}

	ostr << "}\n";
}


/**
 * export flux graph to dot
 * @see https://graphviz.org/doc/info/lang.html
 */
template<class t_graph> requires is_flux_graph<t_graph>
void print_graph(const t_graph& graph, std::ostream& ostr = std::cout)
{
	const std::size_t N = graph.GetNumVertices();

	ostr << "digraph my_graph\n{\n";

	ostr << "\t// vertices\n";
	for(std::size_t i=0; i<N; ++i)
		ostr << "\t" << i << " [label=\"" << graph.GetVertexIdent(i) << "\"];\n";

	ostr << "\n";
	ostr << "\t// edges and weights\n";

	for(std::size_t i=0; i<N; ++i)
	{
		for(std::size_t j=0; j<N; ++j)
		{
			typename t_graph::t_weight f = graph.GetWeight(i, j);
			typename t_graph::t_weight c = graph.GetCapacity(i, j);
			if(!c)
				continue;

			ostr << "\t" << i << " -> " << j << " [label=\"" << f << " / " << c << "\"];\n";
		}
	}

	ostr << "}\n";
}


/**
 * dijkstra algorithm
 * @see (FUH 2021), Kurseinheit 4, p. 17
 */
template<class t_graph> requires is_graph<t_graph>
std::vector<std::optional<std::size_t>>
dijk(const t_graph& graph, const std::string& startvert)
{
	// start index
	auto _startidx = graph.GetVertexIndex(startvert);
	if(!_startidx)
		return {};
	const std::size_t startidx = *_startidx;


	// distances
	const std::size_t N = graph.GetNumVertices();
	using t_weight = typename t_graph::t_weight;

	std::vector<t_weight> dists;
	std::vector<std::optional<std::size_t>> predecessors;
	dists.resize(N);
	predecessors.resize(N);

	// don't use the full maximum to prevent overflows when we're adding the weight afterwards
	const t_weight infinity = std::numeric_limits<t_weight>::max() / 2;
	for(std::size_t vertidx=0; vertidx<N; ++vertidx)
		dists[vertidx] = (vertidx==startidx ? 0 : infinity);


	// distance priority queue and comparator
	auto vert_cmp = [&dists](std::size_t idx1, std::size_t idx2) -> bool
	{
		return dists[idx1] > dists[idx2];
	};

	std::priority_queue<std::size_t, std::vector<std::size_t>, decltype(vert_cmp)>
		prio{vert_cmp};

	for(std::size_t vertidx=0; vertidx<N; ++vertidx)
		prio.push(vertidx);


	while(prio.size())
	{
		std::size_t vertidx = prio.top();
		prio.pop();

		std::vector<std::size_t> neighbours = graph.GetNeighbours(vertidx);
		for(std::size_t neighbouridx : neighbours)
		{
			t_weight w = graph.GetWeight(vertidx, neighbouridx);

			// is the path from startidx to neighbouridx over vertidx shorter than from startidx to neighbouridx?
			if(dists[vertidx] + w < dists[neighbouridx])
			{
				dists[neighbouridx] = dists[vertidx] + w;
				predecessors[neighbouridx] = vertidx;
			}
		}
	}


	/*for(std::size_t i=0; i<N; ++i)
	{
		const std::string& endvert = graph.GetVertexIdent(i);
		std::cout << "dist from " << startvert << " to " << endvert << ": " << dists[i] << std::endl;
	}

	for(std::size_t i=0; i<N; ++i)
	{
		const auto& _predidx = predecessors[i];
		if(!_predidx)
			continue;

		std::size_t predidx = *_predidx;
		const std::string& vert = graph.GetVertexIdent(i);
		const std::string& pred = graph.GetVertexIdent(predidx);

		std::cout << "predecessor of " << vert << ": " << pred << "." << std::endl;
	}*/

	return predecessors;
}


/**
 * bellman-ford algorithm
 * @see (FUH 2021), Kurseinheit 4, p. 13
 */
template<class t_graph, class t_mat=m::mat<typename t_graph::t_weight, std::vector>>
requires is_graph<t_graph> && m::is_mat<t_mat>
t_mat bellman(const t_graph& graph, const std::string& startvert)
{
	// start index
	auto _startidx = graph.GetVertexIndex(startvert);
	if(!_startidx)
		return t_mat{};
	const std::size_t startidx = *_startidx;


	// distances
	const std::size_t N = graph.GetNumVertices();
	using t_weight = typename t_graph::t_weight;
	t_mat dists = m::zero<t_mat>(N, N);

	// don't use the full maximum to prevent overflows when we're adding the weight afterwards
	const t_weight infinity = std::numeric_limits<t_weight>::max() / 2;

	for(std::size_t vertidx=0; vertidx<N; ++vertidx)
		dists(0, vertidx) = (vertidx==startidx ? 0 : infinity);


	// iterate vertices
	for(std::size_t i=1; i<N; ++i)
	{
		for(std::size_t vertidx=0; vertidx<N; ++vertidx)
		{
			dists(i, vertidx) = dists(i-1, vertidx);

			std::vector<std::size_t> neighbours = graph.GetNeighbours(vertidx, false);
			for(std::size_t neighbouridx : neighbours)
			{
				t_weight w = graph.GetWeight(neighbouridx, vertidx);
				//std::cout << "Weight from " << neighbouridx << " to " << vertidx << ": " << w << std::endl;

				if(dists(i-1, neighbouridx) + w < dists(i, vertidx))
				{
					dists(i, vertidx) = dists(i-1, neighbouridx) + w;
				}
			}
		}
	}

	return dists;
}


/**
 * floyd-warshall algorithm
 * @see (FUH 2021), Kurseinheit 4, p. 23
 */
template<class t_graph, class t_mat=m::mat<typename t_graph::t_weight, std::vector>>
requires is_graph<t_graph> && m::is_mat<t_mat>
t_mat floyd(const t_graph& graph)
{
	// distances
	const std::size_t N = graph.GetNumVertices();
	using t_weight = typename t_graph::t_weight;
	t_mat dists = m::zero<t_mat>(N, N);
	t_mat next_dists = m::zero<t_mat>(N, N);


	// don't use the full maximum to prevent overflows when we're adding the weight afterwards
	const t_weight infinity = std::numeric_limits<t_weight>::max() / 2;

	// initial weights
	for(std::size_t vertidx1=0; vertidx1<N; ++vertidx1)
	{
		std::vector<std::size_t> neighbours = graph.GetNeighbours(vertidx1);

		for(std::size_t vertidx2=0; vertidx2<N; ++vertidx2)
		{
			if(vertidx2 == vertidx1)
				continue;

			// is vertidx2 a direct neighbour of vertidx1?
			if(std::find(neighbours.begin(), neighbours.end(), vertidx2) != neighbours.end())
				dists(vertidx1, vertidx2) = graph.GetWeight(vertidx1, vertidx2);
			else
				dists(vertidx1, vertidx2) = infinity;
		}
	}


	// iterate vertices
	for(std::size_t i=1; i<N; ++i)
	{
		for(std::size_t vertidx1=0; vertidx1<N; ++vertidx1)
		{
			for(std::size_t vertidx2=0; vertidx2<N; ++vertidx2)
			{
				t_weight dist1 = dists(vertidx1, vertidx2);
				t_weight dist2 = dists(vertidx1, i) + dists(i, vertidx2);

				next_dists(vertidx1, vertidx2) = std::min(dist1, dist2);
			}
		}

		std::swap(dists, next_dists);
	}

	return dists;
}


/**
 * rest function
 * @see (FUH 2021), Kurseinheit 5, p. 4
 */
template<class t_graph, class t_graph_rest>
	requires is_flux_graph<t_graph> && is_graph<t_graph_rest>
t_graph_rest calc_restflux(const t_graph& graph)
{
	t_graph_rest rest;

	for(std::size_t i=0; i<graph.GetNumVertices(); ++i)
		rest.AddVertex(graph.GetVertexIdent(i));

	for(const auto [vert1, vert2, data] : graph.GetEdges())
	{
		auto weight = std::get<0>(data);
		auto cap = std::get<1>(data);

		rest.AddEdge(vert2, vert1, weight);
		rest.AddEdge(vert1, vert2, cap - weight);
	}

	return rest;
}

#endif
