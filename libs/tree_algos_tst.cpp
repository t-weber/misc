/**
 * tree algo tests
 * @author Tobias Weber
 * @date may-2021
 * @license see 'LICENSE.EUPL' file
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o tree_algos_tst tree_algos_tst.cpp
 * ./tree_algos_tst > 0.dot && dot -Tpdf 0.dot > 0.pdf
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

	avltree_insert<t_nodeptr>(header->right, t_node::create(456));
	avltree_insert<t_nodeptr>(header->right, t_node::create(789));
	avltree_insert<t_nodeptr>(header->right, t_node::create(-321));
	avltree_insert<t_nodeptr>(header->right, t_node::create(-654));
	avltree_insert<t_nodeptr>(header->right, t_node::create(999));
	avltree_insert<t_nodeptr>(header->right, t_node::create(399));
	avltree_insert<t_nodeptr>(header->right, t_node::create(400));
	avltree_insert<t_nodeptr>(header->right, t_node::create(401));
	avltree_insert<t_nodeptr>(header->right, t_node::create(500));
	avltree_insert<t_nodeptr>(header->right, t_node::create(501));
	avltree_insert<t_nodeptr>(header->right, t_node::create(502));

	std::cout << "// sorted values = ";
	bintree_for_each<t_nodeptr>(header->right, [](t_nodeptr node) -> void
	{
		std::cout << node->value << " ";
	});
	std::cout << "\n" << std::endl;

	bintree_print_graph<t_nodeptr>(header->right, std::cout);
	return 0;
}
