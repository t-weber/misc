/**
 * intrusive tree container wrappers
 * @author Tobias Weber
 * @date Nov-2020, May-2021
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *   - https://www.boost.org/doc/libs/1_74_0/doc/html/intrusive/node_algorithms.html
 */

#ifndef __INTRUSIVE_TREE_WRAPPER_H__
#define __INTRUSIVE_TREE_WRAPPER_H__


#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <cstdint>

#include <boost/intrusive/bstree_algorithms.hpp>
#include <boost/intrusive/avltree_algorithms.hpp>
#include <boost/intrusive/rbtree_algorithms.hpp>
#include <boost/intrusive/splaytree_algorithms.hpp>
#include <boost/intrusive/treap_algorithms.hpp>


// ----------------------------------------------------------------------------
// concepts
// ----------------------------------------------------------------------------

template<class T>
concept is_common_tree_node = requires(const T& a)
{
	a.GetParent();
	a.GetLeft();
	a.GetRight();
};


template<class T>
concept is_bin_tree_node = requires(const T& a)
{
	a.GetBalance();
} && is_common_tree_node<T>;


template<class T>
concept is_rb_tree_node = requires(const T& a)
{
	a.GetColour();
} && is_common_tree_node<T>;

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// node types
// ----------------------------------------------------------------------------

template<class t_nodetype>
class CommonTreeNode
{
public:
	CommonTreeNode() {}
	virtual ~CommonTreeNode() {}

	CommonTreeNode(const CommonTreeNode<t_nodetype>& other)
	{
		*this = operator=(other);
	}

	const CommonTreeNode<t_nodetype>& operator=(const CommonTreeNode<t_nodetype>& other)
	{
		this->parent = other.parent;
		this->left = other.left;
		this->right = other.right;
	}

	virtual void PrintValue(std::ostream&) const { }

	t_nodetype* GetParent() { return parent; }
	t_nodetype* GetLeft() { return left; }
	t_nodetype* GetRight() { return right; }

	const t_nodetype* GetParent() const { return parent; }
	const t_nodetype* GetLeft() const { return left; }
	const t_nodetype* GetRight() const { return right; }

	void SetParent(t_nodetype* parent) { this->parent = parent; }
	void SetLeft(t_nodetype* left) { this->left = left; }
	void SetRight(t_nodetype* right) { this->right = right; }

private:
	t_nodetype *parent{nullptr};
	t_nodetype *left{nullptr};
	t_nodetype *right{nullptr};
};


template<class t_nodetype>
class BinTreeNode : public CommonTreeNode<t_nodetype>
{
public:
	using t_balance = std::int64_t;

	BinTreeNode() : CommonTreeNode<t_nodetype>() {};
	virtual ~BinTreeNode() {}

	BinTreeNode(const BinTreeNode<t_nodetype>& other)
	{
		*this = operator=(other);
	}

	const BinTreeNode<t_nodetype>& operator=(const BinTreeNode<t_nodetype>& other)
	{
		static_cast<CommonTreeNode<t_nodetype>*>(this)->operator=(other);
		this->balance = other.balance;
	}

	t_balance GetBalance() const { return balance; }
	void SetBalance(t_balance bal) { this->balance = bal; }

private:
	t_balance balance{0};
};


template<class t_nodetype>
class RbTreeNode : public CommonTreeNode<t_nodetype>
{
public:
	using t_colour = std::int8_t;

	RbTreeNode() : CommonTreeNode<t_nodetype>() {};
	virtual ~RbTreeNode() {}

	RbTreeNode(const RbTreeNode<t_nodetype>& other)
	{
		*this = operator=(other);
	}

	const RbTreeNode<t_nodetype>& operator=(const RbTreeNode<t_nodetype>& other)
	{
		static_cast<CommonTreeNode<t_nodetype>*>(this)->operator=(other);
		this->colour = other.colour;
	}

	t_colour GetColour() const { return colour; }
	void SetColour(t_colour col) { this->colour = col; }

private:
	t_colour colour{0};
};

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// node traits
// ----------------------------------------------------------------------------

/**
 * common node traits
 * @see https://www.boost.org/doc/libs/1_74_0/doc/html/intrusive/node_algorithms.html
 */
template<class t_tree_node> requires is_common_tree_node<t_tree_node>
struct CommonNodeTraits
{
	using node = t_tree_node;
	using node_ptr = node*;
	using const_node_ptr = const node*;

	using t_splaytreealgos = boost::intrusive::splaytree_algorithms<CommonNodeTraits<t_tree_node>>;


	static node* get_parent(const node* thenode)
	{
		if(!thenode)
			return nullptr;
		return const_cast<node*>(thenode->GetParent());
	}

	static node* get_left(const node* thenode)
	{
		if(!thenode)
			return nullptr;
		return const_cast<node*>(thenode->GetLeft());
	}

	static node* get_right(const node* thenode)
	{
		if(!thenode)
			return nullptr;
		return const_cast<node*>(thenode->GetRight());
	}

	static void set_parent(node* thenode, node* parent)
	{
		if(!thenode)
			return;
		thenode->SetParent(parent);
	}

	static void set_left(node* thenode, node* left)
	{
		if(!thenode)
			return;
		thenode->SetLeft(left);
	}

	static void set_right(node* thenode, node* right)
	{
		if(!thenode)
			return;
		thenode->SetRight(right);
	}
};


/**
 * node traits
 * @see https://www.boost.org/doc/libs/1_74_0/doc/html/intrusive/node_algorithms.html
 */
template<class t_tree_node> requires is_bin_tree_node<t_tree_node>
struct BinTreeNodeTraits : public CommonNodeTraits<t_tree_node>
{
	using node = typename CommonNodeTraits<t_tree_node>::node;
	using node_ptr = typename CommonNodeTraits<t_tree_node>::node_ptr;
	using const_node_ptr = typename CommonNodeTraits<t_tree_node>::const_node_ptr;
	using balance = typename node::t_balance;

	using t_avltreealgos = boost::intrusive::avltree_algorithms<BinTreeNodeTraits<t_tree_node>>;
	using t_bstreealgos = boost::intrusive::bstree_algorithms<BinTreeNodeTraits<t_tree_node>>;


	static balance positive() { return 1; }
	static balance negative() { return -1; }
	static balance zero() { return 0; }

	static balance get_balance(const node* thenode)
	{
		if(!thenode)
			return zero();
		return thenode->GetBalance();
	}

	static void set_balance(node* thenode, balance bal)
	{
		if(!thenode)
			return;
		thenode->SetBalance(bal);
	}
};


/**
 * node traits
 * @see https://www.boost.org/doc/libs/1_74_0/doc/html/intrusive/node_algorithms.html
 */
template<class t_tree_node> requires is_rb_tree_node<t_tree_node>
struct RbTreeNodeTraits : public CommonNodeTraits<t_tree_node>
{
	using node = typename CommonNodeTraits<t_tree_node>::node;
	using node_ptr = typename CommonNodeTraits<t_tree_node>::node_ptr;
	using const_node_ptr = typename CommonNodeTraits<t_tree_node>::const_node_ptr;
	using color = typename node::t_colour;

	using t_rbtreealgos = boost::intrusive::rbtree_algorithms<RbTreeNodeTraits<t_tree_node>>;


	static color red() { return 1; }
	static color black() { return 0; }

	static color get_color(const node* thenode)
	{
		if(!thenode)
			return black();
		return thenode->GetColour();
	}

	static void set_color(node* thenode, color col)
	{
		if(!thenode)
			return;
		thenode->SetColour(col);
	}
};

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// functions
// ----------------------------------------------------------------------------

template<class t_node> requires is_common_tree_node<t_node>
void write_graph(std::ostream& ostr, const t_node* node)
{
	std::size_t nodeNum = 0;
	std::unordered_map<const t_node*, std::size_t> nodeNumbers;

	std::ostringstream ostrStates, ostrTransitions;
	ostrStates.precision(ostr.precision());
	ostrTransitions.precision(ostr.precision());


	std::function<void(const t_node*)> _number_nodes;
	_number_nodes = [&_number_nodes, &nodeNum, &nodeNumbers](const t_node* node) -> void
	{
		if(!node)
			return;

		_number_nodes(node->GetLeft());

		if(auto iter = nodeNumbers.find(node); iter == nodeNumbers.end())
			nodeNumbers.emplace(std::make_pair(node, nodeNum++));

		_number_nodes(node->GetRight());
	};


	std::function<void(const t_node*)> _write_graph;
	_write_graph = [&_write_graph, &nodeNumbers, &ostrStates, &ostrTransitions](const t_node* node) -> void
	{
		if(!node)
			return;

		std::size_t num = nodeNumbers.find(node)->second;

		if(node->GetLeft())
		{
			std::size_t numleft = nodeNumbers.find(node->GetLeft())->second;
			ostrTransitions << "\t" << num << ":sw -> " << numleft << ":n [label=\"l\"];\n";

			_write_graph(node->GetLeft());
		}

		ostrStates << "\t" << num << " [label=\"";
		node->PrintValue(ostrStates);
		ostrStates << "\"];\n";

		if(node->GetRight())
		{
			std::size_t numright = nodeNumbers.find(node->GetRight())->second;
			ostrTransitions << "\t" << num << ":se -> " << numright << ":n [label=\"r\"];\n";

			_write_graph(node->GetRight());
		}
	};


	_number_nodes(node);
	_write_graph(node);

	ostr << "// directed graph\ndigraph tree\n{\n\t// states\n";
	ostr << ostrStates.str();
	ostr << "\n\t// transitions\n";
	ostr << ostrTransitions.str();
	ostr << "\n}\n";
}

// ----------------------------------------------------------------------------


#endif
