/**
 * intrusive tree container tests
 * @author Tobias Weber
 * @date May-2021
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *   - https://www.boost.org/doc/libs/1_70_0/doc/html/intrusive/usage.html
 */

#include "intrusive.h"
namespace intr = boost::intrusive;


template<class t_val>
class TreeNode : public BinTreeNode<TreeNode<t_val>>
{
public:
	TreeNode(t_val data=t_val{}) : BinTreeNode<TreeNode<t_val>>(), data{data}
	{}

	virtual ~TreeNode()
	{}

	TreeNode(const TreeNode<t_val>& other)
	{
		*this = operator=(other);
	}

	const TreeNode<t_val>& operator=(const TreeNode<t_val>& other)
	{
		static_cast<BinTreeNode<TreeNode<t_val>>*>(this)->operator=(other);
		this->data = other.data;
	}

	t_val GetData() const
	{
		return data;
	}

	virtual void PrintValue(std::ostream& ostr) const override
	{
		ostr << data;
	}

	static bool compare(const TreeNode<t_val>* node1, const TreeNode<t_val>* node2)
	{
		return *node1 < *node2;
	}

	friend bool operator<(const TreeNode& node1, const TreeNode& node2)
	{
		//std::cout << "compare " << node1.data << " < " << node2.data << "?" << std::endl;
		return node1.GetData() < node2.GetData();
	}

private:
	t_val data{0};
};


int main()
{
	using t_algos = BinTreeNodeTraits<TreeNode<int>>::t_avltreealgos;

	TreeNode<int> root;
	t_algos::init_header(&root);

	t_algos::insert_equal(&root, t_algos::root_node(&root), new TreeNode<int>{123}, &TreeNode<int>::compare);
	t_algos::insert_equal(&root, t_algos::root_node(&root), new TreeNode<int>{456}, &TreeNode<int>::compare);
	t_algos::insert_equal(&root, t_algos::root_node(&root), new TreeNode<int>{789}, &TreeNode<int>::compare);
	t_algos::insert_equal(&root, t_algos::root_node(&root), new TreeNode<int>{-321}, &TreeNode<int>::compare);
	t_algos::insert_equal(&root, t_algos::root_node(&root), new TreeNode<int>{-654}, &TreeNode<int>::compare);


	std::cout << "// linked leaves: ";
	for(TreeNode<int>* iter = t_algos::begin_node(&root); iter != t_algos::end_node(&root); iter = t_algos::next_node(iter))
	{
		std::cout << iter->GetData() << ", ";
	}
	std::cout << "\n" << std::endl;


	write_graph<TreeNode<int>>(std::cout, t_algos::root_node(&root));

	return 0;
}
