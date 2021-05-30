/**
 * graph algorithms
 * @author Tobias Weber
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
#include <iostream>


/**
 * export to dot
 * @see https://graphviz.org/doc/info/lang.html
 */
template<class t_graph>
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
 * dijkstra algorithm
 * @see (FUH 2021), Kurseinheit 4, p. 17
 */
template<class t_graph>
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

	for(std::size_t i=0; i<N; ++i)
		dists[i] = (i==startidx ? 0 : std::numeric_limits<t_weight>::max());


	// distance priority queue and comparator
	auto vert_cmp = [&dists](std::size_t idx1, std::size_t idx2) -> bool
	{
		return dists[idx1] > dists[idx2];
	};

	std::priority_queue<std::size_t, std::vector<std::size_t>, decltype(vert_cmp)>
		prio{vert_cmp};

	for(std::size_t i=0; i<N; ++i)
		prio.push(i);


	while(prio.size())
	{
		std::size_t vertidx = prio.top();
		prio.pop();

		std::vector<std::size_t> neighbours = graph.GetNeighbours(vertidx);
		for(std::size_t neighbouridx : neighbours)
		{
			t_weight w = graph.GetWeight(vertidx, neighbouridx);

			// is the path from s to v over u shorter than from s to v?
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

		std::cout << "predecessor of " << vert << ": " << pred << std::endl;
	}*/

	return predecessors;
}


#endif
