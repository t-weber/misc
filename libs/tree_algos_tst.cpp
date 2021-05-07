/**
 * tree algo tests
 * @author Tobias Weber
 * @date may-2021
 * @license see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o tree_algos_tst tree_algos_tst.cpp
 */

#include "tree_algos.h"


int main()
{
	using t_node = avl_node<int>;
	using t_nodeptr = typename t_node::t_nodeptr;

	auto header = t_node::create(0);
	auto root = t_node::create(123);
	header->right = root;
	root->parent = header;

	bintree_insert<t_nodeptr>(root, t_node::create(456));
	bintree_insert<t_nodeptr>(root, t_node::create(789));
	bintree_insert<t_nodeptr>(root, t_node::create(-321));
	bintree_insert<t_nodeptr>(root, t_node::create(-654));
	bintree_insert<t_nodeptr>(root, t_node::create(999));
	avltree_calc_balances(root);

	root->right = avltree_rotate(root->right, true);
	avltree_calc_balances(root);

	bintree_insert<t_nodeptr>(root, t_node::create(400));
	bintree_insert<t_nodeptr>(root, t_node::create(401));
	bintree_insert<t_nodeptr>(root, t_node::create(500));
	bintree_insert<t_nodeptr>(root, t_node::create(501));
	avltree_calc_balances(root);

	root = avltree_doublerotate(root, true);
	avltree_calc_balances(root);

	bintree_write_graph<t_nodeptr>(std::cout, root);
	return 0;
}
