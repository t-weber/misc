/**
 * graph tests
 * @author Tobias Weber
 * @date may-2021
 * @license see 'LICENSE.EUPL' file
 */

#include "graph_conts.h"


int main()
{
	using t_graph = adjacency_matrix<unsigned int>;

	t_graph graph;

	graph.AddVertex("A");
	graph.AddVertex("B");
	graph.AddVertex("C");
	graph.AddVertex("D");

	graph.AddEdge("A", "B", 2);
	graph.AddEdge("A", "C", 4);
	graph.AddEdge("B", "A", 1);
	graph.AddEdge("B", "D", 10);

	graph.Print(std::cout);
	return 0;
}
