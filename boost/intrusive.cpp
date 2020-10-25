/**
 * Instrusive container tests
 * @author Tobias Weber
 * @date jun-19
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_70_0/doc/html/intrusive/usage.html
 *  * https://www.boost.org/doc/libs/1_70_0/doc/html/intrusive/avl_set_multiset.html
 */

#include <iostream>
#include <memory>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/avltree.hpp>
#include <boost/intrusive/sgtree.hpp>
#include <boost/intrusive/bstree.hpp>
#include <boost/intrusive/rbtree.hpp>

namespace intr = boost::intrusive;


template<class T>
struct LstElem
{
	T val{};
	intr::list_member_hook<intr::link_mode<intr::normal_link>> _h;


	friend std::ostream& operator<<(std::ostream& ostr, const LstElem<T>& e)
	{
		ostr << e.val;
		return ostr;
	}
};


template<class HOOK, class T>
struct TreeLeaf
{
	T val{};
	HOOK _h{};


	friend std::ostream& operator<<(std::ostream& ostr, const TreeLeaf<HOOK, T>& e)
	{
		ostr << e.val;
		return ostr;
	}

	friend bool operator<(const TreeLeaf<HOOK, T>& e1, const TreeLeaf<HOOK, T>& e2)
	{
		//std::cout << "comparing " << e1.val << " and " << e2.val << std::endl;
		return e1.val < e2.val;
	}
};


template<class T>
using AVLLeaf = TreeLeaf<intr::avl_set_member_hook<intr::link_mode<intr::normal_link>>, T>;

template<class T>
using BSLeaf = TreeLeaf<intr::bs_set_member_hook<intr::link_mode<intr::normal_link>>, T>;


template<class t_elem, class node_traits, class t_node_ptr>
void print_tree(t_node_ptr node, unsigned depth=0)
{
	using t_hook = decltype(t_elem::_h);

	// get parent object pointer from hook member
	const t_elem *elem = 
		intr::get_parent_from_member<t_elem, t_hook>
		(reinterpret_cast<t_hook*>(node), &t_elem::_h);
		//reinterpret_cast<t_elem*>(reinterpret_cast<char*>(node) - 8);

	for(unsigned i=0; i<depth; ++i)
		std::cout << "\t";
	std::cout << "value: " << elem->val << std::endl;

	auto left = node_traits::get_left(node);
	auto right = node_traits::get_right(node);

	if(left) print_tree<t_elem, node_traits>(left, depth+1);
	if(right) print_tree<t_elem, node_traits>(right, depth+1);
}


template<class t_val, class t_tree, template<class...> class t_leaf>
void test_tree()
{
	t_tree tree;
	using node_traits = typename t_tree::node_traits;

	t_leaf<t_val> e1{.val = 10};
	t_leaf<t_val> e2{.val = 5};
	t_leaf<t_val> e3{.val = 15};
	t_leaf<t_val> e4{.val = 2};
	t_leaf<t_val> e5{.val = 15};
	t_leaf<t_val> e6{.val = 30};
	t_leaf<t_val> e7{.val = 4};

	//tree.insert_unique(e1);
	tree.insert_equal(e1);
	tree.insert_equal(e2);
	tree.insert_equal(e3);
	tree.insert_equal(e4);
	tree.insert_equal(e5);
	tree.insert_equal(e6);
	tree.insert_equal(e7);

	//std::cout << "addresses: " << (void*)&e1 << ", " << (void*)&e2 << std::endl;
	t_leaf<t_val> e1b{.val = 10};
	std::cout << "find(10): " << tree.find(e1b)->val << std::endl;
	std::cout << "lower_bound(10): " << tree.lower_bound(e1b)->val << std::endl;
	std::cout << "upper_bound(10): " << tree.upper_bound(e1b)->val << std::endl;


	for(const auto& elem : tree)
	{
		std::cout << "element: "
			<< (void*)&elem << ", hook: "
			<< (void*)&elem._h
			<< ", value: " << elem << std::endl;
	}

	print_tree<t_leaf<int>, node_traits>(tree.root()->_h.this_ptr());


	bool closedrange1 = true;
	bool closedrange2 = true;
	auto [iter1, iter2] = tree.bounded_range(
		t_leaf<t_val>{.val = 10}, t_leaf<t_val>{.val = 15}, 
		closedrange1, closedrange2);
	std::cout << "nodes in range [10, 15]: ";
	for(auto iter=iter1; iter!=iter2; ++iter)
		std::cout << *iter << ", ";
	std::cout << std::endl;


	std::cout << "Erasing 15:" << std::endl;
	tree.erase(t_leaf<t_val>{.val = 15});
	print_tree<t_leaf<t_val>, node_traits>(tree.root()->_h.this_ptr());

	/*auto iterEnd = tree.begin();
	++iterEnd;
	tree.erase(tree.begin(), iterEnd);
	print_tree<t_leaf<t_val>, node_traits>(tree.root()->_h.this_ptr());*/
}


int main()
{
	// list
	{
		intr::list<LstElem<int>,
			//intr::link_mode<intr::safe_link>,
			//intr::link_mode<intr::auto_unlink>,
			intr::member_hook<LstElem<int>, decltype(LstElem<int>::_h), &LstElem<int>::_h>> lst;

		LstElem<int> e1{.val = 1};
		LstElem<int> e2{.val = 2};
		lst.push_back(e1);
		lst.push_back(e2);

		std::cout << "addresses: " << (void*)&e1 << ", " << (void*)&e2 << std::endl;

		for(const auto& elem : lst)
		{
			std::cout << "element: " 
				<< (void*) &elem << ": " 
				<< elem << std::endl;
		}

		// clear list in case safe_link is used
		//lst.clear();
	}


	std::cout << std::endl;


	// binary trees
	{
		using t_val = int;
		using t_avl = intr::avltree<AVLLeaf<t_val>,
			intr::member_hook<AVLLeaf<t_val>, decltype(AVLLeaf<t_val>::_h), &AVLLeaf<t_val>::_h>>;
		using t_sg = intr::sgtree<BSLeaf<t_val>,
			intr::member_hook<BSLeaf<t_val>, decltype(BSLeaf<t_val>::_h), &BSLeaf<t_val>::_h>>;
		using t_bs = intr::bstree<BSLeaf<t_val>,
			intr::member_hook<BSLeaf<t_val>, decltype(BSLeaf<t_val>::_h), &BSLeaf<t_val>::_h>>;
		//using t_rb = intr::rbtree<BSLeaf<t_val>,
		//	intr::member_hook<BSLeaf<t_val>, decltype(BSLeaf<t_val>::_h), &BSLeaf<t_val>::_h>>;

		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		std::cout << "AVL tree" << std::endl;
		test_tree<t_val, t_avl, AVLLeaf>();
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		std::cout << std::endl;

		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		std::cout << "SG tree" << std::endl;
		test_tree<t_val, t_sg, BSLeaf>();
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		std::cout << std::endl;

		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		std::cout << "BS tree" << std::endl;
		test_tree<t_val, t_bs, BSLeaf>();
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		//std::cout << std::endl;

		//std::cout << "--------------------------------------------------------------------------------" << std::endl;
		//std::cout << "RB tree" << std::endl;
		//test_tree<t_val, t_rb, BSLeaf>();
		//std::cout << "--------------------------------------------------------------------------------" << std::endl;
	}


	return 0;
}
