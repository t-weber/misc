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

namespace intr = boost::intrusive;


template<class T>
struct LstElem
{
	T val;
	intr::list_member_hook<intr::link_mode<intr::normal_link>> _h;

	friend std::ostream& operator<<(std::ostream& ostr, const LstElem<T>& e)
	{
		ostr << e.val;
		return ostr;
	}
};


template<class T>
struct AVLElem
{
	T val;
	intr::avl_set_member_hook<intr::link_mode<intr::normal_link>> _h;

	friend std::ostream& operator<<(std::ostream& ostr, const AVLElem<T>& e)
	{
		ostr << e.val;
		return ostr;
	}

	friend bool operator<(const AVLElem<T>& e1, const AVLElem<T>& e2)
	{
		return e1.val < e2.val;
	}
};


template<class node_traits, class t_node_ptr>
void recurse_avl(t_node_ptr node)
{
	auto left = node_traits::get_left(node);
	auto right = node_traits::get_right(node);

	if(left) recurse_avl<node_traits>(left);
	if(right) recurse_avl<node_traits>(right);
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
			std::cout << "element: " << (void*) &elem << ": " << elem << std::endl;
		}

		// clear list in case safe_link is used
		//lst.clear();
	}


	std::cout << std::endl;


	// AVL tree
	{
		intr::avltree<AVLElem<int>,
			intr::member_hook<AVLElem<int>, decltype(AVLElem<int>::_h), &AVLElem<int>::_h>> avl;
		using node_traits = decltype(avl)::node_traits;
		using value_traits = decltype(avl)::value_traits;

		AVLElem<int> e1{.val = 10};
		AVLElem<int> e2{.val = 5};
		AVLElem<int> e3{.val = 15};
		AVLElem<int> e4{.val = 2};
		AVLElem<int> e5{.val = 15};
		//avl.insert_unique(e1);
		avl.insert_equal(e1);
		avl.insert_equal(e2);
		avl.insert_equal(e3);
		avl.insert_equal(e4);
		avl.insert_equal(e5);

		//std::cout << "addresses: " << (void*)&e1 << ", " << (void*)&e2 << std::endl;
		//AVLElem<int> e1b{.val = 10};
		//std::cout << (void*)&*avl.find(e1b) << std::endl;


		auto root = avl.root();
		//std::cout << "root value: " << root->val << std::endl;
		//std::cout << "left hook: " << node_traits::get_left(root->_h.this_ptr()) << std::endl;
		recurse_avl<node_traits>(root->_h.this_ptr());


		for(const auto& elem : avl)
		{
			std::cout << "element: " << (void*)&elem << ", hook: " << (void*)&elem._h << ", value: " << elem << std::endl;
		}
	}


	return 0;
}