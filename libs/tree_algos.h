/**
 * tree algos
 * @author Tobias Weber
 * @date may-2021
 * @license see 'LICENSE.EUPL' file
 */

#ifndef __TREE_ALGOS_H__
#define __TREE_ALGOS_H__

#include <memory>
#include <functional>
#include <unordered_map>
#include <sstream>
#include <iostream>



// ----------------------------------------------------------------------------
// concepts
// ----------------------------------------------------------------------------

template<class T>
concept is_node = requires(const T& a)
{
	a.parent;
	a.left;
	a.right;

	a.value;
};


template<class T>
concept is_avl_node = requires(const T& a)
{
	a.balance;
} && is_node<T>;

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// data types
// ----------------------------------------------------------------------------

template<class t_val>
struct avl_node
{
	using t_nodeptr = std::shared_ptr<avl_node>;
	using t_constnodeptr = std::shared_ptr<const avl_node>;

	t_nodeptr parent{};
	t_nodeptr left{};
	t_nodeptr right{};
	int balance{};

	t_val value{};


	static t_nodeptr create(t_val val = 0)
	{
		auto node = std::make_shared<avl_node<t_val>>();
		node->value = val;
		return node;
	}
};

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// algorithms
// ----------------------------------------------------------------------------

template<class t_nodeptr> requires is_node<decltype(*t_nodeptr{})>
void bintree_insert(t_nodeptr root, t_nodeptr node)
{
	if(node->value < root->value)
	{
		if(!root->left)
			root->left = node;
		else
			bintree_insert<t_nodeptr>(root->left, node);
	}
	else
	{
		if(!root->right)
			root->right = node;
		else
			bintree_insert<t_nodeptr>(root->right, node);
	}
}


template<class t_nodeptr> requires is_node<decltype(*t_nodeptr{})>
void bintree_write_graph(std::ostream& ostr, t_nodeptr node)
{
	std::size_t nodeNum = 0;
	std::unordered_map<t_nodeptr, std::size_t> nodeNumbers;

	std::ostringstream ostrStates;
	std::ostringstream ostrTransitions;
	ostrStates.precision(ostr.precision());
	ostrTransitions.precision(ostr.precision());


	std::function<void(t_nodeptr)> _number_nodes;
	_number_nodes = [&_number_nodes, &nodeNum, &nodeNumbers](t_nodeptr node) -> void
	{
		if(!node)
			return;

		_number_nodes(node->left);

		if(auto iter = nodeNumbers.find(node); iter == nodeNumbers.end())
			nodeNumbers.emplace(std::make_pair(node, nodeNum++));

		_number_nodes(node->right);
	};


	std::function<void(t_nodeptr)> _write_graph;
	_write_graph = [&_write_graph, &nodeNumbers, &ostrStates, &ostrTransitions](t_nodeptr node) -> void
	{
		if(!node)
			return;

		std::size_t num = nodeNumbers.find(node)->second;

		if(node->left)
		{
			std::size_t numleft = nodeNumbers.find(node->left)->second;
			ostrTransitions << "\t" << num << ":sw -> " << numleft << ":n [label=\"l\"];\n";

			_write_graph(node->left);
		}

		ostrStates << "\t" << num << " [label=\"" << node->value << "\"];\n";

		if(node->right)
		{
			std::size_t numright = nodeNumbers.find(node->right)->second;
			ostrTransitions << "\t" << num << ":se -> " << numright << ":n [label=\"r\"];\n";

			_write_graph(node->right);
		}
	};


	_number_nodes(node);
	_write_graph(node);

	ostr << "// directed graph\n" << "digraph tree\n{";
	ostr << "\n\t// states\n";
	ostr << ostrStates.str();
	ostr << "\n\t// transitions\n";
	ostr << ostrTransitions.str();
	ostr << "\n}\n";
}

// ----------------------------------------------------------------------------

#endif
