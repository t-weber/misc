/**
 * C++ tests
 * @author Tobias Weber
 * @date oct-20
 * @license: see 'LICENSE.EUPL' file
 */

#include <vector>
#include <iostream>
#include <iterator>


template<class t_cont>
class circular_iterator : public std::iterator<
	typename std::iterator_traits<typename t_cont::iterator>::iterator_category,
	typename t_cont::value_type,
	typename t_cont::difference_type,
	typename t_cont::pointer,
	typename t_cont::reference>
{
public:
	using t_iter = typename t_cont::iterator;

public:
	circular_iterator(t_cont& cont, t_iter iter, std::size_t round=0)
		: m_cont(cont), m_iter(iter), m_round{round}
	{}

	std::size_t GetRound() const { return m_round; }

	typename t_cont::reference operator*() { return *m_iter; }


	bool operator==(circular_iterator iter) const
	{ return this->m_iter.operator==(iter.m_iter); }

	bool operator!=(circular_iterator iter) const
	{ return this->m_iter.operator!=(iter.m_iter); }

	//bool operator<(circular_iterator iter) const
	//{ return this.m_iter.operator<(iter.m_iter); }


	circular_iterator& operator++()
	{
		std::advance(m_iter, 1);
		if(m_iter == m_cont.end())
		{
			// wrap around
			m_iter = m_cont.begin();
			++m_round;
		}
		return *this;
	}

	circular_iterator& operator--()
	{
		if(m_iter == m_cont.begin())
		{
			// wrap around
			m_iter = std::prev(m_cont.end(), 1);
			++m_round;
		}
		else
		{
			std::advance(m_iter, -1);
		}
		return *this;
	}


	circular_iterator operator++(int)
	{
		auto curiter = *this;
		this->operator++();
		return curiter;
	}


	circular_iterator operator--(int)
	{
		auto curiter = *this;
		this->operator--();
		return curiter;
	}


	circular_iterator& operator+=(std::size_t num)
	{
		for(std::size_t i=0; i<num; ++i)
			operator++();
		return *this;
	}

	circular_iterator& operator-=(std::size_t num)
	{
		for(std::size_t i=0; i<num; ++i)
			operator--();
		return *this;
	}


	circular_iterator operator+(std::size_t num)
	{
		auto iter = *this;
		iter.operator+=(num);
		return iter;
	}



private:
	t_cont& m_cont;
	t_iter m_iter;

	// how often has the container range been looped?
	std::size_t m_round{0};
};



/**
 * circular access to a container
 */
template<class t_cont>
class circular_wrapper
{
public:
	using iterator = circular_iterator<t_cont>;

public:
	circular_wrapper(t_cont& cont) : m_cont{cont}
	{}


	iterator begin() { return iterator(m_cont, m_cont.begin()); }
	iterator end() { return iterator(m_cont, m_cont.end()); }


	typename t_cont::reference operator[](std::size_t i)
	{ return *(begin() + i); }

	const typename t_cont::reference operator[](std::size_t i) const
	{ return *(begin() + i); }

private:
	t_cont& m_cont;
};


int main()
{
	std::vector<int>v {{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }};

	auto iter1 = v.rbegin();
	auto iter2 = iter1 + 5;

	std::cout << *iter1 << " " << *iter2 << std::endl;

	//v.erase(iter2.base(), std::next(iter1, 1).base());
	v.erase(iter2.base(), iter1.base());
	for(int i : v)
		std::cout << i << ", ";
	std::cout << std::endl;


	circular_wrapper circ(v);
	for(auto iter = circ.begin(); iter.GetRound()<5; std::advance(iter, 1))
		std::cout << *iter << ", round: " << iter.GetRound() << std::endl;

	circular_wrapper circ2(v);
	for(std::size_t i=0; i<50; ++i)
		std::cout << circ2[i] << ", ";
	std::cout << std::endl;
	return 0;
}
