/**
 * memory allocation test
 * @author Tobias Weber
 * @date aug-2020
 * @license: see 'LICENSE.EUPL' file
 */

#include <iostream>
#include <iterator>
#include <list>
#include <vector>
#include <algorithm>


struct Seg
{
	std::size_t start;
	std::size_t size;

	bool operator== (const Seg& seg) const
	{
		return start==seg.start && size==seg.size;
	}
};



class VariSeg
{
public:
	VariSeg(std::size_t memsize) : m_memsize{memsize}
	{
	}


	/**
	 * allocates a segment in the first free gap
	 */
	const Seg* AllocFirstFree(std::size_t size)
	{
		Seg seg{ .start=0, .size=size };

		// this is the first segment
		if(!m_segs.size())
		{
			if(size <= m_memsize)
				return &m_segs.emplace_back(std::move(seg));
			else
				return nullptr;
		}

		// space before first segment
		if(m_segs.size() && size <= m_segs.begin()->size)
			return &*m_segs.emplace(m_segs.begin(), std::move(seg));

		for(auto iter=m_segs.begin(); iter!=m_segs.end(); std::advance(iter, 1))
		{
			std::size_t curstart = iter->start;
			std::size_t cursize = iter->size;
			std::size_t nextstart = 0;

			auto iterNext = std::next(iter, 1);
			if(iterNext == m_segs.end())
				nextstart = m_memsize;
			else
				nextstart = iterNext->start;

			// find a gap between segments
			std::size_t gap = nextstart - (curstart+cursize);
			if(gap < size)
				continue;

			seg.start = curstart+cursize;
			return &*m_segs.emplace(iterNext, std::move(seg));
		}

		return nullptr;
	}


	/**
	 * allocates a segment in the largest (or smallest) free gap
	 */
	const Seg* AllocLargestFree(std::size_t size, bool find_smallest=false)
	{
		Seg seg{ .start=0, .size=size };

		// this is the first segment
		if(!m_segs.size())
		{
			if(size <= m_memsize)
				return &m_segs.emplace_back(std::move(seg));
			else
				return nullptr;
		}

		// get possible candidates
		std::vector<std::tuple<Seg, typename decltype(m_segs)::iterator>> candidates;

		// space before first segment
		if(m_segs.size() && size <= m_segs.begin()->size)
			candidates.emplace_back(std::make_tuple(Seg{.start = 0, .size=m_segs.begin()->size}, m_segs.begin()));

		for(auto iter=m_segs.begin(); iter!=m_segs.end(); std::advance(iter, 1))
		{
			std::size_t curstart = iter->start;
			std::size_t cursize = iter->size;
			std::size_t nextstart = 0;

			auto iterNext = std::next(iter, 1);
			if(iterNext == m_segs.end())
				nextstart = m_memsize;
			else
				nextstart = iterNext->start;

			// find a gap between segments
			std::size_t gap = nextstart - (curstart+cursize);
			if(gap < size)
				continue;

			candidates.emplace_back(std::make_tuple(Seg{.start = curstart+cursize, .size=gap}, iterNext));
		}

		if(candidates.size() == 0)
			return nullptr;

		// sort candidates
		std::stable_sort(candidates.begin(), candidates.end(),
			[find_smallest](const auto& seg1, const auto& seg2) -> bool
		{
			if(find_smallest)
				return std::get<0>(seg1).size < std::get<0>(seg2).size;
			else
				return std::get<0>(seg1).size > std::get<0>(seg2).size;
		});

		seg.start = std::get<0>(*candidates.begin()).start;
		return &*m_segs.emplace(std::get<1>(*candidates.begin()), std::move(seg));
	}


	/**
	 * removes a segment
	 */
	void Free(std::size_t start)
	{
		for(Seg& seg : m_segs)
		{
			if(seg.start == start)
			{
				m_segs.remove(seg);
				break;
			}
		}
	}


	/**
	 * calculates external fragmentation: sizes between segments
	 */
	std::size_t GetFrag() const
	{
		std::size_t frag = 0;

		for(auto iter=m_segs.begin(); iter!=m_segs.end(); std::advance(iter, 1))
		{
			auto iterNext = std::next(iter, 1);
			if(iterNext == m_segs.end())
				break;

			// gap between 0 and the first segment
			if(iter == m_segs.begin())
				frag += iter->start;

			// gap size between segments
			std::size_t gap = iterNext->start - (iter->start+iter->size);
			frag += gap;
		}

		return frag;
	}


	/**
	 * calculates total free memory
	 */
	std::size_t GetFree() const
	{
		std::size_t free = 0;

		for(auto iter=m_segs.begin(); iter!=m_segs.end(); std::advance(iter, 1))
		{
			std::size_t curstart = iter->start;
			std::size_t cursize = iter->size;
			std::size_t nextstart = 0;

			auto iterNext = std::next(iter, 1);
			if(iterNext == m_segs.end())
				nextstart = m_memsize;
			else
				nextstart = iterNext->start;

			// gap between 0 and the first segment
			if(iter == m_segs.begin())
				free += iter->start;

			// get gap between segments
			free += nextstart - (curstart+cursize);
		}

		return free;
	}


protected:
	std::size_t m_memsize{};
	std::list<Seg> m_segs{};
};



int main()
{
	{
		VariSeg mem(1000);
		std::vector<const Seg*> segs;

		for(int i=0; i<10; ++i)
		{
			const Seg* seg = mem.AllocFirstFree(100);
			segs.push_back(seg);

			std::cout << "seg: " << seg->start << ", free: " << mem.GetFree()
				<< ", frag: " << mem.GetFrag() << std::endl;
		}

		mem.Free(segs[5]->start);
		std::cout << "free: " << mem.GetFree() << ", frag: " << mem.GetFrag() << std::endl;

		const Seg* seg = mem.AllocFirstFree(100);
		std::cout << "seg: " << seg->start << ", free: " << mem.GetFree()
			<< ", frag: " << mem.GetFrag() << std::endl;
	}
	std::cout << std::endl;

	{
		VariSeg mem(1000);

		const Seg* seg1 = mem.AllocLargestFree(100);
		std::cout << "seg 1: " << seg1->start << ", free: " << mem.GetFree()
			<< ", frag: " << mem.GetFrag() << std::endl;

		const Seg* seg2 = mem.AllocLargestFree(200);
		std::cout << "seg 2: " << seg2->start << ", free: " << mem.GetFree()
			<< ", frag: " << mem.GetFrag() << std::endl;

		const Seg* seg3 = mem.AllocLargestFree(200);
		std::cout << "seg 3: " << seg3->start << ", free: " << mem.GetFree()
			<< ", frag: " << mem.GetFrag() << std::endl;

		mem.Free(seg1->start);
		std::cout << "free: " << mem.GetFree() << ", frag: " << mem.GetFrag() << std::endl;

		const Seg* seg4 = mem.AllocLargestFree(100, true);
		std::cout << "seg 4: " << seg4->start << ", free: " << mem.GetFree()
			<< ", frag: " << mem.GetFrag() << std::endl;
	}

	return 0;
}
