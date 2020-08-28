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

	// linear position in memory
	std::size_t lin_pos = 0;

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


	std::tuple<bool, std::size_t> allocate(std::size_t size)
	{
		std::size_t allocsize = nextpow2(size);
		return alloc_node(m_node.get(), allocsize, size);
	}


	void deallocate(std::size_t linpos)
	{
		// if there's only the root node, deallocate it by setting the used size to 0
		if(m_node->lin_pos == linpos && m_node->used_size != 0)
			m_node->used_size = 0;
		else
			dealloc_node(m_node.get(), linpos);
	}


	std::tuple<std::size_t, std::size_t> get_free_and_frag() const
	{
		const auto [total_alloc, actual_alloc] = get_allocated(m_node.get());
		std::size_t free = m_node->level_size - total_alloc;
		std::size_t frag = total_alloc - actual_alloc;

		return std::make_tuple(free, frag);
	}


protected:
	static std::tuple<bool, std::size_t>
	alloc_node(MemNode* node, std::size_t allocsize, std::size_t actualsize)
	{
		if(node->level_size < allocsize)
		{
			// not enough space on this level
			return std::make_tuple(false, 0);
		}
		else if(node->level_size == allocsize && node->used_size == 0
			&& !node->children[0] && !node->children[1])
		{
			// found fitting node
			node->used_size = actualsize;
			return std::make_tuple(true, node->lin_pos);
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
					node->children[child]->lin_pos = node->lin_pos + child*node->children[child]->level_size;
				}

				if(auto tup = alloc_node(node->children[child].get(), allocsize, actualsize);
					std::get<0>(tup))
					return tup;
			}
		}

		return std::make_tuple(false, 0);
	}


	static void dealloc_node(MemNode* node, std::size_t linpos)
	{
		for(int child=0; child<2; ++child)
		{
			if(node->children[child]->lin_pos == linpos)
			{
				if(node->children[child]->used_size == 0)
				{
					// not yet at leaf node
					dealloc_node(node->children[child].get(), linpos);
				}
				else
				{
					// at leaf node
					node->children[child].reset();
				}
			}
		}
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

	std::size_t seg2 = 0;

	{
		auto tup = seg.allocate(500);
		std::cout << std::boolalpha << std::get<0>(tup) << ", lin pos: " << std::get<1>(tup) << ", ";
		auto [free, frag] = seg.get_free_and_frag();
		std::cout << "free: " << free << ", int frag: " << frag << std::endl;
	}
	{
		auto tup = seg.allocate(200);
		std::cout << std::boolalpha << std::get<0>(tup) << ", lin pos: " << std::get<1>(tup) << ", ";
		seg2 = std::get<1>(tup);
		auto [free, frag] = seg.get_free_and_frag();
		std::cout << "free: " << free << ", int frag: " << frag << std::endl;
	}
	{
		auto tup = seg.allocate(200);
		std::cout << std::boolalpha << std::get<0>(tup) << ", lin pos: " << std::get<1>(tup) << ", ";
		auto [free, frag] = seg.get_free_and_frag();
		std::cout << "free: " << free << ", int frag: " << frag << std::endl;
	}
	{
		auto tup = seg.allocate(200);
		std::cout << std::boolalpha << std::get<0>(tup) << ", lin pos: " << std::get<1>(tup) << ", ";
		auto [free, frag] = seg.get_free_and_frag();
		std::cout << "free: " << free << ", int frag: " << frag << std::endl;
	}
	{
		seg.deallocate(seg2);
		auto [free, frag] = seg.get_free_and_frag();
		std::cout << "free: " << free << ", int frag: " << frag << std::endl;
	}
	{
		auto tup = seg.allocate(200);
		std::cout << std::boolalpha << std::get<0>(tup) << ", lin pos: " << std::get<1>(tup) << ", ";
		auto [free, frag] = seg.get_free_and_frag();
		std::cout << "free: " << free << ", int frag: " << frag << std::endl;
	}
	{
		auto tup = seg.allocate(200);
		std::cout << std::boolalpha << std::get<0>(tup) << ", lin pos: " << std::get<1>(tup) << ", ";
		auto [free, frag] = seg.get_free_and_frag();
		std::cout << "free: " << free << ", int frag: " << frag << std::endl;
	}

	return 0;
}
