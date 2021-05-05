/**
 * intrusive tree container tests
 * @author Tobias Weber
 * @date May-2021
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *   - https://www.boost.org/doc/libs/1_76_0/doc/html/intrusive/usage.html
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o intrusive intrusive.cpp
 */

//#define _INTR_USE_SHARED_PTR
#include "intrusive.h"
namespace intr = boost::intrusive;


template<class t_val>
class TreeNode : public BinTreeNode<TreeNode<t_val>>
{
public:
	TreeNode(t_val m_data=t_val{}) : BinTreeNode<TreeNode<t_val>>(), m_data{m_data}
	{}

	virtual ~TreeNode()
	{
		//std::cout << __PRETTY_FUNCTION__ << ", val = " << m_data << std::endl;
	}

	TreeNode(const TreeNode<t_val>& other)
	{
		*this = operator=(other);
	}

	const TreeNode<t_val>& operator=(const TreeNode<t_val>& other)
	{
		static_cast<BinTreeNode<TreeNode<t_val>>*>(this)->operator=(other);
		this->m_data = other.m_data;
	}

	t_val GetData() const
	{
		return m_data;
	}

	virtual void PrintValue(std::ostream& ostr) const override
	{
		ostr << m_data << " (balance: " << BinTreeNode<TreeNode<t_val>>::GetBalance() << ")";
	}


	static typename TreeNode<t_val>::t_nodeptr create(t_val val=0)
	{
		#ifdef _INTR_USE_SHARED_PTR
		return std::make_shared<TreeNode<t_val>>(val);
		#else
		return new TreeNode<t_val>(val);
		#endif
	}

	static bool compare(
		const typename TreeNode<t_val>::t_nodeptr node1,
		const typename TreeNode<t_val>::t_nodeptr node2)
	{
		return *node1 < *node2;
	}

	friend bool operator<(const TreeNode& node1, const TreeNode& node2)
	{
		//std::cout << "compare " << node1.m_data << " < " << node2.m_data << "?" << std::endl;
		return node1.GetData() < node2.GetData();
	}

private:
	t_val m_data{0};
};


int main()
{
	using t_node = TreeNode<int>;
	using t_algos = BinTreeNodeTraits<t_node>::t_avltreealgos;

	auto tree = t_node::create();
	t_algos::init_header(tree);

	t_algos::insert_equal(tree, t_algos::root_node(tree), t_node::create(123), &t_node::compare);
	t_algos::insert_equal(tree, t_algos::root_node(tree), t_node::create(456), &t_node::compare);
	t_algos::insert_equal(tree, t_algos::root_node(tree), t_node::create(789), &t_node::compare);
	t_algos::insert_equal(tree, t_algos::root_node(tree), t_node::create(-321), &t_node::compare);
	t_algos::insert_equal(tree, t_algos::root_node(tree), t_node::create(-654), &t_node::compare);

	std::cout << "// linked leaves: ";
	for(auto iter = tree->GetLeft(); iter; iter = t_algos::next_node(iter))
	{
		std::cout << iter->GetData() << ", ";
		if(iter == tree->GetRight())
			break;
	}
	std::cout << "\n" << std::endl;

	// write graph diagram
	if(t_algos::root_node(tree) != tree)
		write_graph<t_node>(std::cout, t_algos::root_node(tree));

#ifndef _INTR_USE_SHARED_PTR
	if(t_algos::root_node(tree) != tree)
		free_nodes<t_node, t_algos>(t_algos::root_node(tree));
#endif

	//std::cout << "end of " << __PRETTY_FUNCTION__ << std::endl;
	return 0;
}
