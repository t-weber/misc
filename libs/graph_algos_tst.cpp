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


int main()
{
	using t_graph = adjacency_matrix<unsigned int>;

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
	dijk<t_graph>(graph, "A");

	return 0;
}
