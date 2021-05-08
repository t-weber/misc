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
	//a.height;
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
	//unsigned int height{};

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

/**
 * insert a node in a binary tree
 */
template<class t_nodeptr> requires is_node<decltype(*t_nodeptr{})>
void bintree_insert(t_nodeptr root, t_nodeptr node)
{
	if(node->value < root->value)
	{
		if(!root->left)
		{
			root->left = node;
			node->parent = root->left;
		}
		else
		{
			bintree_insert<t_nodeptr>(root->left, node);
		}
	}
	else
	{
		if(!root->right)
		{
			root->right = node;
			node->parent = root->right;
		}
		else
		{
			bintree_insert<t_nodeptr>(root->right, node);
		}
	}

	//bintree_set_parents<t_nodeptr>(root);
}


/**
 * insert a node in an avl tree
 * @see https://en.wikipedia.org/wiki/AVL_tree
 */
template<class t_nodeptr> requires is_avl_node<decltype(*t_nodeptr{})>
void avltree_insert(t_nodeptr root, t_nodeptr node)
{
	t_nodeptr head = root->parent;

	if(node->value < root->value)
	{
		if(!root->left)
		{
			root->left = node;
			node->parent = root->left;
		}
		else
		{
			bintree_insert<t_nodeptr>(root->left, node);
		}
	}
	else
	{
		if(!root->right)
		{
			root->right = node;
			node->parent = root->right;
		}
		else
		{
			bintree_insert<t_nodeptr>(root->right, node);
		}
	}

	bintree_set_parents<t_nodeptr>(head);
	avltree_calc_balances(head);

	std::function<void(t_nodeptr)> _balance_nodes;
	_balance_nodes = [&_balance_nodes, &root](t_nodeptr node) -> void
	{
		if(!node)
			return;

		// try all rotations
		node->right = avltree_doublerotate<t_nodeptr>(root, node->right, true);
		node->left = avltree_doublerotate<t_nodeptr>(root, node->left, true);
		node->right = avltree_doublerotate<t_nodeptr>(root, node->right, false);
		node->left = avltree_doublerotate<t_nodeptr>(root, node->left, false);

		node->right = avltree_rotate<t_nodeptr>(root, node->right, true);
		node->left = avltree_rotate<t_nodeptr>(root, node->left, true);
		node->right = avltree_rotate<t_nodeptr>(root, node->right, false);
		node->left = avltree_rotate<t_nodeptr>(root, node->left, false);

		if(node->parent)
			_balance_nodes(node->parent);
	};

	_balance_nodes(node->parent);
}


/**
 * correctly set the parent pointers
 */
template<class t_nodeptr> requires is_node<decltype(*t_nodeptr{})>
void bintree_set_parents(t_nodeptr node, t_nodeptr parent=nullptr)
{
	if(parent)
		node->parent = parent;

	if(node->left)
		bintree_set_parents<t_nodeptr>(node->left, node);
	if(node->right)
		bintree_set_parents<t_nodeptr>(node->right, node);
}


/**
 * write the tree out as directed graph
 */
template<class t_nodeptr> requires is_node<decltype(*t_nodeptr{})>
void bintree_print_graph(t_nodeptr node, std::ostream& ostr = std::cout)
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

		ostrStates << "\t" << num << " [label=\"" << node->value;
		if constexpr(is_avl_node<decltype(*t_nodeptr{})>)
			ostrStates << " (balance: " << node->balance << ")";
		ostrStates << "\"];\n";

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


/**
 * print all nodes in linear order
 */
template<class t_nodeptr> requires is_node<decltype(*t_nodeptr{})>
void bintree_print_linear(t_nodeptr thenode, std::ostream& ostr = std::cout)
{
	std::function<void(t_nodeptr)> _print;
	_print = [&_print, &ostr](t_nodeptr node) -> void
	{
		if(node->left)
			_print(node->left);

		ostr << node->value << " ";

		if(node->right)
			_print(node->right);
	};

	_print(thenode);
}


/**
 * execute function "func" for all nodes in linear order
 */
template<class t_nodeptr, class t_func>
requires is_node<decltype(*t_nodeptr{})>
void bintree_for_each(t_nodeptr thenode, t_func func)
{
	std::function<void(t_nodeptr)> _print;
	_print = [&_print, &func](t_nodeptr node) -> void
	{
		if(node->left)
			_print(node->left);

		func(node);

		if(node->right)
			_print(node->right);
	};

	_print(thenode);
}


/**
 * calculate avl tree balance factors
 * @see https://en.wikipedia.org/wiki/AVL_tree
 */
template<class t_nodeptr> requires is_avl_node<decltype(*t_nodeptr{})>
void avltree_calc_balances(t_nodeptr node)
{
	std::function<std::size_t(t_nodeptr)> _get_height;
	_get_height = [&_get_height](t_nodeptr node) -> std::size_t
	{
		std::size_t heightLeft = node->left ? _get_height(node->left) + 1 : 0;
		std::size_t heightRight = node->right ? _get_height(node->right) + 1 : 0;
		std::size_t height = std::max(heightLeft, heightRight);
		node->balance = heightRight - heightLeft;
		//node->left->height = heightLeft;
		//node->right->height = heightRight;
		//node->height = height;

		return height;
	};

	_get_height(node);
}


/**
 * calculate avl tree balance factors
 * @see https://en.wikipedia.org/wiki/AVL_tree
 */
template<class t_nodeptr> requires is_avl_node<decltype(*t_nodeptr{})>
t_nodeptr avltree_rotate(t_nodeptr root, t_nodeptr node, bool rot_left)
{
	if(!node)
		return node;
	t_nodeptr head = root->parent;

	t_nodeptr new_root = node;

	// left rotation
	if(rot_left && node->right && node->parent && node->balance == 2 && node->right->balance >= 0)
	{
		t_nodeptr parent = node->parent;
		bool node_is_left_child = (parent->left == node);

		t_nodeptr right = node->right;
		if(node_is_left_child)
			parent->left = right;
		else
			parent->right = right;

		node->right = right->left;
		right->left = node;

		new_root = right;
	}

	// right rotation
	else if(!rot_left && node->left && node->parent && node->balance == -2 && node->left->balance <= 0)
	{
		t_nodeptr parent = node->parent;
		bool node_is_right_child = (parent->right == node);

		t_nodeptr left = node->left;
		if(node_is_right_child)
			parent->right = left;
		else
			parent->left = left;

		node->left = left->right;
		left->right = node;

		new_root = left;
	}
	else
	{
		return node;
	}

	bintree_set_parents<t_nodeptr>(head);
	avltree_calc_balances<t_nodeptr>(head);

	return new_root;
}


/**
 * calculate avl tree balance factors
 * @see https://en.wikipedia.org/wiki/AVL_tree
 */
template<class t_nodeptr> requires is_avl_node<decltype(*t_nodeptr{})>
t_nodeptr avltree_doublerotate(t_nodeptr root, t_nodeptr node, bool rot_rightleft)
{
	if(!node)
		return node;

	// rl rotation
	if(rot_rightleft && node->balance == 2 && node->right->balance <= 0)
	{
		node->right = avltree_rotate(root, node->right, false);
		node = avltree_rotate(root, node, true);
	}

	// lr rotation
	else if(!rot_rightleft && node->left && node->balance == -2 && node->left->balance >= 0)
	{
		node->left = avltree_rotate(root, node->left, true);
		node = avltree_rotate(root, node, false);
	}

	return node;
}


// ----------------------------------------------------------------------------

#endif
