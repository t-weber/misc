/**
 * memory allocation test
 * @author Tobias Weber
 * @date aug-2020
 * @license: see 'LICENSE.EUPL' file
 */

#include <memory>
#include <iostream>


template<class t_int=std::size_t>
t_int nextpow2(t_int num)
{
	int highest_bit = -1;

	for(int bit=sizeof(num)*8-1; bit>=0; --bit)
	{
		if(num & (1<<bit))
		{
			highest_bit = bit;
			break;
		}
	}

	t_int curpow2 = 1<<highest_bit;
	if(curpow2 == num)
		return curpow2;
	else
		return curpow2 << 1;
}


struct MemNode
{
	std::size_t level_size = 0;
	std::size_t used_size = 0;

	std::unique_ptr<MemNode> children[2];
};


class Segment
{
public:
	Segment(std::size_t memsize)
	{
		m_node = std::make_unique<MemNode>();
		m_node->level_size = memsize;
	}


	bool allocate(std::size_t size)
	{
		std::size_t allocsize = nextpow2(size);
		return alloc_node(m_node.get(), allocsize, size);
	}


	std::tuple<std::size_t, std::size_t> get_free_and_frag() const
	{
		const auto [total_alloc, actual_alloc] = get_allocated(m_node.get());
		std::size_t free = m_node->level_size - total_alloc;
		std::size_t frag = total_alloc - actual_alloc;

		return std::make_tuple(free, frag);
	}


protected:
	static bool alloc_node(MemNode* node, std::size_t allocsize, std::size_t actualsize)
	{
		if(node->level_size < allocsize)
		{
			// not enough space on this level
			return false;
		}
		else if(node->level_size == allocsize && node->used_size == 0
			&& !node->children[0] && !node->children[1])
		{
			// found fitting node
			node->used_size = actualsize;
			return true;
		}
		else if(node->level_size > allocsize && node->used_size == 0)
		{
			// try next level
			for(int child=0; child<2; ++child)
			{
				if(!node->children[child])
				{
					node->children[child] = std::make_unique<MemNode>();
					node->children[child]->level_size = node->level_size>>1;
				}

				if(alloc_node(node->children[child].get(), allocsize, actualsize))
					return true;
			}
		}

		return false;
	}


	std::tuple<std::size_t, std::size_t> get_allocated(const MemNode* node) const
	{
		if(node->used_size)
		{
			return std::make_tuple(node->level_size, node->used_size);
		}
		else
		{
			std::size_t total_alloc = 0;
			std::size_t actual_alloc = 0;

			for(int child=0; child<2; ++child)
			{
				if(!node->children[child])
					continue;

				auto tup = get_allocated(node->children[child].get());
				total_alloc += std::get<0>(tup);
				actual_alloc += std::get<1>(tup);
			}

			return std::make_tuple(total_alloc, actual_alloc);
		}
	}


private:
	std::unique_ptr<MemNode> m_node{};
};



int main()
{
	Segment seg(1024);
	std::cout << std::boolalpha << seg.allocate(500) << std::endl;
	std::cout << std::boolalpha << seg.allocate(200) << std::endl;
	std::cout << std::boolalpha << seg.allocate(200) << std::endl;
	std::cout << std::boolalpha << seg.allocate(200) << std::endl;

	auto [free, frag] = seg.get_free_and_frag();
	std::cout << "free: " << free << ", int frag: " << frag << std::endl;

	return 0;
}
