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


template<class t_graph> requires is_graph<t_graph>
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
	auto predecessors_mod = dijk_mod<t_graph>(graph, "A");
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

	std::cout << "\ndijkstra (mod):" << std::endl;
	for(std::size_t i=0; i<graph.GetNumVertices(); ++i)
	{
		const auto& _predidx = predecessors_mod[i];
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


template<class t_flux_graph, class t_graph>
	requires is_flux_graph<t_flux_graph> && is_graph<t_graph>
void tst_flux()
{
	using namespace m_ops;
	t_flux_graph graph;

	graph.AddVertex("A");
	graph.AddVertex("B");
	graph.AddVertex("C");
	graph.AddVertex("D");
	graph.AddVertex("E");

	graph.AddEdge("A", "B", 2);  graph.SetCapacity("A", "B", 3);
	graph.AddEdge("A", "C", 4);  graph.SetCapacity("A", "C", 4);
	graph.AddEdge("B", "C", 10); graph.SetCapacity("B", "C", 15);
	graph.AddEdge("B", "D", 3);  graph.SetCapacity("B", "D", 5);
	graph.AddEdge("C", "D", 1);  graph.SetCapacity("C", "D", 2);

	t_graph rest = calc_restflux<t_flux_graph, t_graph>(graph);
	t_flux_graph maxflux = flux_max<t_flux_graph, t_graph>(graph, "A", "D");

	std::cout << "graph:" << std::endl;
	print_graph(graph, std::cout);

	std::cout << "\nrest graph:" << std::endl;
	print_graph(rest, std::cout);

	std::cout << "\nflux maximum:" << std::endl;
	print_graph(maxflux, std::cout);
}


int main()
{
	{
		std::cout << "using adjacency matrix" << std::endl;
		using t_graph = adjacency_matrix<unsigned int>;
		tst<t_graph>();
	}

	std::cout << "\n--------------------------------------------------------------------------------" << std::endl;

	{
		std::cout << "\nusing adjacency list" << std::endl;
		using t_graph = adjacency_list<unsigned int>;
		tst<t_graph>();
	}

	std::cout << "\n--------------------------------------------------------------------------------" << std::endl;

	{
		std::cout << "\nflux graph" << std::endl;
		using t_flux_graph = adjacency_matrix<std::pair<unsigned int, unsigned int>, unsigned int>;
		using t_graph = adjacency_matrix<unsigned int>;
		tst_flux<t_flux_graph, t_graph>();
	}
	return 0;
}
