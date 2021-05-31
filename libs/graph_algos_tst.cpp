/**
 * graph tests
 * @author Tobias Weber
 * @date may-2021
 * @license see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o graph_algos_tst graph_algos_tst.cpp
 */

#include "graph_conts.h"
#include "graph_algos.h"


template<class t_graph>
void tst()
{
	using namespace m_ops;
	t_graph graph;

	graph.AddVertex("A");
	graph.AddVertex("B");
	graph.AddVertex("C");
	graph.AddVertex("D");
	graph.AddVertex("E");

	graph.AddEdge("A", "B", 2);
	graph.AddEdge("A", "C", 4);
	graph.AddEdge("B", "A", 1);
	graph.AddEdge("B", "D", 10);
	graph.AddEdge("D", "E", 3);
	graph.AddEdge("C", "E", 1);

	print_graph<t_graph>(graph, std::cout);
	auto predecessors = dijk<t_graph>(graph, "A");
	auto distvecs = bellman<t_graph>(graph, "A");
	auto distvecs2 = floyd<t_graph>(graph);

	std::cout << "\ndijkstra:" << std::endl;
	for(std::size_t i=0; i<graph.GetNumVertices(); ++i)
	{
		const auto& _predidx = predecessors[i];
		if(!_predidx)
			continue;

		std::size_t predidx = *_predidx;
		const std::string& vert = graph.GetVertexIdent(i);
		const std::string& pred = graph.GetVertexIdent(predidx);

		std::cout << "predecessor of " << vert << ": " << pred << "." << std::endl;
	}

	std::cout << "\nbellman:" << std::endl;
	std::cout << distvecs << std::endl;


	std::cout << "\nfloyd:" << std::endl;
	std::cout << distvecs2 << std::endl;

}


int main()
{
	{
		std::cout << "using adjacency matrix" << std::endl;
		using t_graph = adjacency_matrix<unsigned int>;
		tst<t_graph>();
	}

	{
		std::cout << "\n\nusing adjacency list" << std::endl;
		using t_graph = adjacency_list<unsigned int>;
		tst<t_graph>();
	}
	return 0;
}
