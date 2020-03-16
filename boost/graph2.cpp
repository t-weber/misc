/**
 * graph tests
 * @author Tobias Weber
 * @date 03-dec-17
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://github.com/boostorg/graph/tree/develop/example
 *  * http://www.boost.org/doc/libs/1_65_1/libs/graph/doc/table_of_contents.html
 *
 * gcc -o graph graph.cpp -std=c++17 -lstdc++ -lm
 */

#include <iostream>
#include <fstream>
#include <string>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>



// ----------------------------------------------------------------------------
// types
using t_real = double;

using t_col = boost::default_color_type;

struct t_vertex
{
	std::string name;

	t_vertex() = default;
	t_vertex(const std::string& strName) : name(strName) {}
};

struct t_edge
{
	std::string name;

	t_edge() = default;
	t_edge(const std::string& strName) : name(strName) {}
};

using t_graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, t_vertex, t_edge>;
using t_vertex_iter = boost::graph_traits<t_graph>::vertex_iterator;
using t_edge_descr = boost::graph_traits<t_graph>::edge_descriptor;
// ----------------------------------------------------------------------------



int main()
{
	t_graph graph;


	// access to members
	auto vert_idx = boost::get(boost::vertex_index, graph);
	auto vertexname = boost::get(&t_vertex::name, graph);
	auto edgename = boost::get(&t_edge::name, graph);


	std::vector<t_vertex> verts = {
		t_vertex("Test 1"),
		t_vertex("Test 2"),
		t_vertex("Test 3"),
		t_vertex("Test 4"),
		t_vertex("Test 5"),
	};

	// add vertices
	for(const auto& vert : verts)
		boost::add_vertex(vert, graph);


	// add edges
	if(auto [edge, bInserted] = boost::add_edge(boost::vertex(0,graph), boost::vertex(1,graph), t_edge("E0"), graph); !bInserted)
		std::cout << "Edge 0 not inserted!" << std::endl;
	if(auto [edge, bInserted] = boost::add_edge(boost::vertex(1,graph), boost::vertex(2,graph), t_edge("E1"), graph); !bInserted)
		std::cout << "Edge 1 not inserted!" << std::endl;
	if(auto [edge, bInserted] = boost::add_edge(boost::vertex(2,graph), boost::vertex(3,graph), t_edge("E2"), graph); !bInserted)
		std::cout << "Edge 2 not inserted!" << std::endl;
	if(auto [edge, bInserted] = boost::add_edge(boost::vertex(3,graph), boost::vertex(4,graph), t_edge("E3"), graph); !bInserted)
		std::cout << "Edge 3 not inserted!" << std::endl;
	if(auto [edge, bInserted] = boost::add_edge(boost::vertex(4,graph), boost::vertex(2,graph), t_edge("E4"), graph); !bInserted)
		std::cout << "Edge 4 not inserted!" << std::endl;


	// write graph to an svg file
	std::map<std::string, std::string> mapGraph, mapVertex, mapEdge;
	std::ofstream ofgraph("tst.graph");
	boost::write_graphviz(ofgraph, graph,
		boost::make_label_writer(vertexname),	// vertex name
		boost::make_label_writer(edgename),	// edge name
		boost::make_graph_attributes_writer(mapGraph, mapVertex, mapEdge));
	std::system("dot -Tsvg tst.graph -o tst.svg");

	return 0;
}
