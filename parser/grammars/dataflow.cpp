/**
 * data flow analysis
 * @author Tobias Weber
 * @date 12-jan-20
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- "Übersetzerbau" (1999, 2013), ISBN: 978-3540653899, Chapter 8.2
 */

#include <memory>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>


/**
 * graph node
 */
struct Node
{
	std::vector<std::shared_ptr<Node>> pred;
	std::vector<std::shared_ptr<Node>> succ;

	std::string id;

	Node(const char* id) : id(id) {}
};


/**
 * add a (directed) edge between two graph vertices
 */
void add_edge(std::shared_ptr<Node> n1, std::shared_ptr<Node> n2)
{
	n1->succ.push_back(n2);
	n2->pred.push_back(n1);
}


std::vector<std::set<int>> out_set_backward_iter(
	const std::vector<std::shared_ptr<Node>>& nodes,
	const std::vector<std::set<int>>& in_set)
{
	std::vector<std::set<int>> out_set_new;
	out_set_new.reserve(in_set.size());

	for(const auto& node : nodes)
	{
		std::set<int> out_set;

		// iterate node's successors
		for(const auto& succ : node->succ)
		{
			auto iterNode = find_if(nodes.begin(), nodes.end(),
				[&succ](auto iter) -> bool
				{
					return iter->id == succ->id;
				});

			std::ptrdiff_t succIdx = iterNode - nodes.begin();
			out_set.insert(in_set[succIdx].begin(), in_set[succIdx].end());
		}

		out_set_new.push_back(out_set);
	}

	return out_set_new;
}


std::vector<std::set<int>> in_set_forward_iter(
	const std::vector<std::shared_ptr<Node>>& nodes,
	const std::vector<std::set<int>>& out_set)
{
	std::vector<std::set<int>> in_set_new;
	in_set_new.reserve(out_set.size());

	for(const auto& node : nodes)
	{
		std::set<int> in_set;

		// iterate node's predecessors
		for(const auto& pred : node->pred)
		{
			auto iterNode = find_if(nodes.begin(), nodes.end(),
				[&pred](auto iter) -> bool
				{
					return iter->id == pred->id;
				});

			std::ptrdiff_t predIdx = iterNode - nodes.begin();
			in_set.insert(out_set[predIdx].begin(), out_set[predIdx].end());
		}

		in_set_new.push_back(in_set);
	}

	return in_set_new;
}


std::vector<std::set<int>> in_set_backward_iter(
	const std::vector<std::shared_ptr<Node>>& nodes,
	const std::vector<std::set<int>>& gen_set,
        const std::vector<std::set<int>>& kill_set,
	const std::vector<std::set<int>>& out_set)
{
	std::vector<std::set<int>> in_set_new;
	in_set_new.reserve(out_set.size());

	for(std::size_t i=0; i<out_set.size(); ++i)
	{
		const auto& out_elem = out_set[i];
		const auto& gen_elem = gen_set[i];
		const auto& kill_elem = kill_set[i];

		std::set<int> in_set = out_elem;
		for(const auto& eraseelem : kill_elem)
			in_set.erase(eraseelem);
		in_set.insert(gen_elem.begin(), gen_elem.end());

		in_set_new.push_back(in_set);
	}

	return in_set_new;
}


std::vector<std::set<int>> out_set_forward_iter(
	const std::vector<std::shared_ptr<Node>>& nodes,
	const std::vector<std::set<int>>& gen_set,
        const std::vector<std::set<int>>& kill_set,
	const std::vector<std::set<int>>& in_set)
{
	std::vector<std::set<int>> out_set_new;
	out_set_new.reserve(in_set.size());

	for(std::size_t i=0; i<in_set.size(); ++i)
	{
		const auto& in_elem = in_set[i];
		const auto& gen_elem = gen_set[i];
		const auto& kill_elem = kill_set[i];

		std::set<int> out_set = in_elem;
		for(const auto& eraseelem : kill_elem)
			out_set.erase(eraseelem);
		out_set.insert(gen_elem.begin(), gen_elem.end());

		out_set_new.push_back(out_set);
	}

	return out_set_new;
}



void print_inout(
	std::size_t iteration,
	const std::vector<std::shared_ptr<Node>>& nodes,
	const std::vector<std::set<int>>& in_set,
	const std::vector<std::set<int>>& out_set)
{
	std::cout << "Iteration " << iteration << "\n\n";

	std::cout << std::setw(10) << std::left << "Node"
		<< std::setw(30) << std::left << "In"
		<< std::setw(30) << std::left << "Out" << "\n";

	for(std::size_t i=0; i<in_set.size(); ++i)
	{
		const auto& node = nodes[i];
		const auto& in_elem = in_set[i];
		const auto& out_elem = out_set[i];

		std::ostringstream ostrIn, ostrOut;
		ostrIn << "{ "; ostrOut << "{ ";
		for(const auto& theelem : in_elem)
			ostrIn << theelem << " ";
		for(const auto& theelem : out_elem)
			ostrOut << theelem << " ";
		ostrIn << "}"; ostrOut << "}";

		std::cout << std::setw(10) << std::left << node->id
			<< std::setw(30) << std::left << ostrIn.str()
			<< std::setw(30) << std::left << ostrOut.str();

		std::cout << "\n";
	}

	std::cout << "\n" << std::endl;
}


void backward_analysis(
	const std::vector<std::shared_ptr<Node>>& nodes,
	const std::vector<std::set<int>>& gen_set,
	const std::vector<std::set<int>>& kill_set)
{
	std::vector<std::set<int>> out_set;
	out_set.resize(nodes.size());

	std::vector<std::set<int>> in_set = gen_set;

	std::size_t iteration = 1;
	while(1)
	{
		print_inout(iteration, nodes, in_set, out_set);

		auto out_set_new = out_set_backward_iter(nodes, in_set);
		// no more changes
		if(out_set == out_set_new)
			break;

		out_set = out_set_new;
		auto in_set_new = in_set_backward_iter(nodes, gen_set, kill_set, out_set);

		in_set = in_set_new;

		++iteration;
	}
}


void forward_analysis(
	const std::vector<std::shared_ptr<Node>>& nodes,
	const std::vector<std::set<int>>& gen_set,
	const std::vector<std::set<int>>& kill_set)
{
	std::vector<std::set<int>> in_set;
	in_set.resize(nodes.size());

	std::vector<std::set<int>> out_set = gen_set;

	std::size_t iteration = 1;
	while(1)
	{
		print_inout(iteration, nodes, in_set, out_set);

		auto in_set_new = in_set_forward_iter(nodes, out_set);
		// no more changes
		if(in_set == in_set_new)
			break;
		in_set = in_set_new;

		auto out_set_new = out_set_forward_iter(nodes, gen_set, kill_set, in_set);
		out_set = out_set_new;

		++iteration;
	}
}


int main()
{
	std::vector<std::shared_ptr<Node>> nodes =
	{{
		std::make_shared<Node>("B1"),
		std::make_shared<Node>("B2"),
		std::make_shared<Node>("B3"),
		std::make_shared<Node>("B4"),
	}};

	add_edge(nodes[0], nodes[1]);
	add_edge(nodes[1], nodes[2]);
	add_edge(nodes[2], nodes[1]);
	add_edge(nodes[2], nodes[3]);


	std::vector<std::set<int>> gen_set =
	{
		{ 1, 33 },	// B1
		{ },		// B2
		{ 2 },		// B3
		{ },		// B4
	};

	std::vector<std::set<int>> kill_set =
	{
		{ 22 },		// B1
		{ },		// B2
		{ 1 },		// B3
		{ },		// B4
	};


	std::cout << "Backward analysis\n\n";
	backward_analysis(nodes, gen_set, kill_set);

	std::cout << "\n\nForward analysis\n\n";
	forward_analysis(nodes, gen_set, kill_set);

	return 0;
}
