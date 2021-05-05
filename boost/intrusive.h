/**
 * intrusive tree container wrappers
 * @author Tobias Weber
 * @date Nov-2020, May-2021
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *   - https://www.boost.org/doc/libs/1_76_0/doc/html/intrusive/node_algorithms.html
 */

#ifndef __INTRUSIVE_TREE_WRAPPER_H__
#define __INTRUSIVE_TREE_WRAPPER_H__


#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
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
#ifdef _INTR_USE_SHARED_PTR
	using t_nodeptr = std::shared_ptr<t_nodetype>;
	using t_constnodeptr = std::shared_ptr<const t_nodetype>;
#else
	using t_nodeptr = std::remove_const_t<t_nodetype>*;
	using t_constnodeptr = const t_nodetype*;
#endif

public:
	CommonTreeNode()
	{
	}

	virtual ~CommonTreeNode()
	{
		//std::cout << __PRETTY_FUNCTION__ << std::endl;
	}

	CommonTreeNode(const CommonTreeNode<t_nodetype>& other)
	{
		*this = operator=(other);
	}

	const CommonTreeNode<t_nodetype>& operator=(const CommonTreeNode<t_nodetype>& other)
	{
		this->m_parent = other.m_parent;
		this->m_left = other.m_left;
		this->m_right = other.m_right;
	}

	virtual void PrintValue(std::ostream&) const { }

	t_nodeptr GetParent() const { return m_parent; }
	t_nodeptr GetLeft() const { return m_left; }
	t_nodeptr GetRight() const { return m_right; }

	void SetParent(t_nodeptr parent) { this->m_parent = parent; }
	void SetLeft(t_nodeptr left) { this->m_left = left; }
	void SetRight(t_nodeptr right) { this->m_right = right; }

private:
	t_nodeptr m_parent{nullptr};
	t_nodeptr m_left{nullptr};
	t_nodeptr m_right{nullptr};
};


template<class t_nodetype>
class BinTreeNode : public CommonTreeNode<t_nodetype>
{
public:
	using t_balance = std::int64_t;

public:
	BinTreeNode() : CommonTreeNode<t_nodetype>() {};
	virtual ~BinTreeNode() {}

	BinTreeNode(const BinTreeNode<t_nodetype>& other)
	{
		*this = operator=(other);
	}

	const BinTreeNode<t_nodetype>& operator=(const BinTreeNode<t_nodetype>& other)
	{
		static_cast<CommonTreeNode<t_nodetype>*>(this)->operator=(other);
		this->m_balance = other.m_balance;
	}

	t_balance GetBalance() const { return m_balance; }
	void SetBalance(t_balance bal) { this->m_balance = bal; }

private:
	t_balance m_balance{0};
};


template<class t_nodetype>
class RbTreeNode : public CommonTreeNode<t_nodetype>
{
public:
	using t_colour = std::int8_t;

public:
	RbTreeNode() : CommonTreeNode<t_nodetype>() {};
	virtual ~RbTreeNode() {}

	RbTreeNode(const RbTreeNode<t_nodetype>& other)
	{
		*this = operator=(other);
	}

	const RbTreeNode<t_nodetype>& operator=(const RbTreeNode<t_nodetype>& other)
	{
		static_cast<CommonTreeNode<t_nodetype>*>(this)->operator=(other);
		this->m_colour = other.m_colour;
	}

	t_colour GetColour() const { return m_colour; }
	void SetColour(t_colour col) { this->m_colour = col; }

private:
	t_colour m_colour{0};
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
	using node_ptr = typename t_tree_node::t_nodeptr;
	using const_node_ptr = typename t_tree_node::t_constnodeptr;

	using t_splaytreealgos = boost::intrusive::splaytree_algorithms<CommonNodeTraits<t_tree_node>>;


	static node_ptr get_parent(const_node_ptr thenode)
	{
		if(!thenode)
			return nullptr;
		//return const_cast<node_ptr>(thenode->GetParent());
		return thenode->GetParent();
	}

	static node_ptr get_left(const_node_ptr thenode)
	{
		if(!thenode)
			return nullptr;
		//return const_cast<node_ptr>(thenode->GetLeft());
		return thenode->GetLeft();
	}

	static node_ptr get_right(const_node_ptr thenode)
	{
		if(!thenode)
			return nullptr;
		//return const_cast<node_ptr>(thenode->GetRight());
		return thenode->GetRight();
	}

	static void set_parent(node_ptr thenode, node_ptr parent)
	{
		if(!thenode)
			return;
		thenode->SetParent(parent);
	}

	static void set_left(node_ptr thenode, node_ptr left)
	{
		if(!thenode)
			return;
		thenode->SetLeft(left);
	}

	static void set_right(node_ptr thenode, node_ptr right)
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

	static balance get_balance(const_node_ptr thenode)
	{
		if(!thenode)
			return zero();
		return thenode->GetBalance();
	}

	static void set_balance(node_ptr thenode, balance bal)
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

	static color get_color(const_node_ptr thenode)
	{
		if(!thenode)
			return black();
		return thenode->GetColour();
	}

	static void set_color(node_ptr thenode, color col)
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
template<class t_node, class t_algos>
requires is_common_tree_node<t_node>
void free_nodes(typename CommonNodeTraits<t_node>::node_ptr node)
{
	using node_ptr = typename CommonNodeTraits<t_node>::node_ptr;

	//std::unordered_set<node_ptr> alreadySeen;
	std::function<void(node_ptr)> _free_nodes;
	_free_nodes = [&_free_nodes/*, &alreadySeen*/](node_ptr node) -> void
	{
		if(!node)
			return;
		/*if(alreadySeen.find(node) != alreadySeen.end())
			return;
		alreadySeen.insert(node);*/

		_free_nodes(node->GetLeft());
		_free_nodes(node->GetRight());

		//std::cout << "removing " << std::hex << (void*)node.get() << std::endl;
		//t_algos::erase(t_algos::get_header(node), node);
		t_algos::unlink(node);

#ifndef _INTR_USE_SHARED_PTR
		delete node;
#endif
	};

	_free_nodes(node);
}


template<class t_node> requires is_common_tree_node<t_node>
void write_graph(std::ostream& ostr, typename CommonNodeTraits<t_node>::const_node_ptr node)
{
	using const_node_ptr = typename CommonNodeTraits<t_node>::const_node_ptr;

	std::size_t nodeNum = 0;
	std::unordered_map<const_node_ptr, std::size_t> nodeNumbers;

	std::ostringstream ostrStates, ostrTransitions;
	ostrStates.precision(ostr.precision());
	ostrTransitions.precision(ostr.precision());


	std::function<void(const_node_ptr)> _number_nodes;
	_number_nodes = [&_number_nodes, &nodeNum, &nodeNumbers](const_node_ptr node) -> void
	{
		if(!node)
			return;

		_number_nodes(node->GetLeft());

		if(auto iter = nodeNumbers.find(node); iter == nodeNumbers.end())
			nodeNumbers.emplace(std::make_pair(node, nodeNum++));

		_number_nodes(node->GetRight());
	};


	std::function<void(const_node_ptr)> _write_graph;
	_write_graph = [&_write_graph, &nodeNumbers, &ostrStates, &ostrTransitions](const_node_ptr node) -> void
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
