/**
 * raw vector test
 * @author Tobias Weber
 * @date aug-2026
 * @license: see 'LICENSE.GPL' file
 */

// clang++ -Wall -Wextra -Weffc++ -std=c++20 -o rawvec rawvec.cpp


#include <type_traits>
#include <iostream>
#include <vector>
#include <memory>



/**
 * generic iterator
 * @see https://en.cppreference.com/cpp/iterator/iterator
 */
template<class t_vec_, bool is_const = false>
struct gen_iterator
{
	using t_vec = std::conditional<is_const, const t_vec_, t_vec_>::type;
	using value_type = typename t_vec_::value_type;
	using size_type = typename t_vec_::size_type;
	using difference_type = typename t_vec_::difference_type;
	using pointer = std::conditional<is_const, const value_type*, value_type*>::type;
	using reference = std::conditional<is_const, const value_type&, value_type&>::type;

	explicit gen_iterator(t_vec *vec, size_type idx) : m_vec{vec}, m_idx{idx}
	{ }

	bool operator==(gen_iterator<t_vec_, is_const> iter) const
	{
		return m_vec == iter.m_vec && m_idx == iter.m_idx;
	}

	bool operator!=(gen_iterator<t_vec_, is_const> iter) const
	{
		return !operator==(iter);
	}

	reference operator*() const
	{
		return (*m_vec)[m_idx];
	}

	template<bool is_var = !is_const, typename = std::enable_if_t<is_var>>
	value_type& operator*()
	{
		return (*m_vec)[m_idx];
	}

	gen_iterator<t_vec_, is_const>& operator++()
	{
		++m_idx;
		return *this;
	}

	gen_iterator<t_vec_, is_const>& operator+=(size_type num)
	{
		m_idx += num;
		return *this;
	}

	gen_iterator<t_vec_, is_const> operator++(int)
	{
		gen_iterator<t_vec_, is_const> prev = *this;
		operator++();
		return prev;
	}


private:
	t_vec* m_vec{};
	size_type m_idx{};
};



/**
 * vector with raw memory management and static and dynamic components
 */
template<class T = double, std::size_t STATIC_SIZE = 4>
class vec_raw
{
public:
	using value_type = T;

	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using iterator = gen_iterator<vec_raw<T, STATIC_SIZE>, false>;
	using const_iterator = gen_iterator<vec_raw<T, STATIC_SIZE>, true>;


public:
	vec_raw()
	{
		std::cout << "Constructed vector, this = " << (void*)this << std::endl;
	}


	~vec_raw()
	{
		clear();

		std::cout << "Destroyed vector, this = " << (void*)this << std::endl;
	}


	vec_raw(vec_raw<T, STATIC_SIZE>&& other) noexcept
		: m_data{ std::exchange(other.m_data, nullptr) },
			m_size{ std::exchange(other.m_size, 0) },
			m_reserve{ std::exchange(other.m_reserve, 0) }
	{
		for(size_type idx = 0; idx < std::min(STATIC_SIZE, m_size); ++idx)
			m_static_data[idx] = other.m_static_data[idx];

		std::cout << "Moved vector, this = " << (void*)this << ", other = " << (void*)&other << std::endl;
	}


	vec_raw(const vec_raw<T, STATIC_SIZE>& other)
	{
		resize(other.size());
		for(size_type idx = 0; idx < other.size(); ++idx)
			(*this)[idx] = other[idx];

		std::cout << "Copied vector, this = " << (void*)this << ", other = " << (void*)&other << std::endl;
	}


	template<class T_other, std::size_t STATIC_SIZE_OTHER>
	vec_raw(const vec_raw<T_other, STATIC_SIZE_OTHER>& other)
	{
		operator=<T_other, STATIC_SIZE_OTHER>(other);

		std::cout << "Copied vector from different type, this = " << (void*)this << ", other = " << (void*)&other << std::endl;
	}


	vec_raw(const std::initializer_list<T>& lst)
	{
		resize(lst.size());
		size_type idx = 0;
		for(const T& elem : lst)
			(*this)[idx++] = elem;

		std::cout << "Constructed vector from list, this = " << (void*)this << std::endl;
	}


	vec_raw(size_type SIZE)
	{
		std::cout << "Constructed vector with size " << SIZE << ", this = " << (void*)this << std::endl;

		resize(SIZE);
	}


	vec_raw<T, STATIC_SIZE>& operator=(vec_raw<T, STATIC_SIZE>&& other)
	{
		m_data = std::exchange(other.m_data, nullptr);
		m_size = std::exchange(other.m_size, 0);
		m_reserve = std::exchange(other.m_reserve, 0);

		for(size_type idx = 0; idx < std::min(STATIC_SIZE, m_size); ++idx)
			m_static_data[idx] = other.m_static_data[idx];

		std::cout << "moved vector, this = " << (void*)this << ", other = " << (void*)&other << std::endl;

		return *this;
	}


	vec_raw<T, STATIC_SIZE>& operator=(const vec_raw<T, STATIC_SIZE>& other)
	{
		resize(other.size());
		for(size_type idx = 0; idx < other.size(); ++idx)
			(*this)[idx] = other[idx];

		std::cout << "copied vector, this = " << (void*)this << ", other = " << (void*)&other << std::endl;

		return *this;
	}


	template<class T_other, std::size_t STATIC_SIZE_OTHER>
	vec_raw<T, STATIC_SIZE>& operator=(const vec_raw<T_other, STATIC_SIZE_OTHER>& other)
	{
		resize(other.size());
		for(size_type idx = 0; idx < other.size(); ++idx)
			(*this)[idx] = other[idx];

		std::cout << "copied vector from different type, this = " << (void*)this << ", other = " << (void*)&other << std::endl;

		return *this;
	}


	void push_back(const T& t)
	{
		size_type num = size();
		resize(num + 1);
		(*this)[num] = t;
	}


	void emplace_back(T&& t)
	{
		size_type num = size();
		resize(num + 1);
		(*this)[num] = std::forward<T>(t);
	}


	void clear()
	{
		if(m_data)
		{
			std::cout << "Freeing " << (void*)m_data << ", this = " << (void*)this << std::endl;

			std::free(reinterpret_cast<void*>(m_data));
			m_data = nullptr;
		}

		m_size = m_reserve = 0;
	}


	void resize(size_type size)
	{
		if(size == m_size)
			return;

		if(size <= m_reserve)
		{
			m_size = size;
			return;
		}

		if(size > STATIC_SIZE)
		{
			const size_type dyn_size = size - STATIC_SIZE;
			alloc(dyn_size);
		}

		m_size = size;
		if(m_size > m_reserve)
			m_reserve = m_size;
	}


	void reserve(size_type size)
	{
		if(size <= m_size || size <= m_reserve)
			return;

		if(size > STATIC_SIZE)
		{
			const size_type dyn_size = size - STATIC_SIZE;
			alloc(dyn_size);
		}

		m_reserve = size;
	}


	size_type size() const { return m_size; }
	iterator begin() { return iterator{this, 0}; }
	iterator end() { return iterator(this, m_size); }
	const_iterator begin() const { return const_iterator(this, 0); }
	const_iterator end() const { return const_iterator(this, m_size); }


	value_type& operator[](size_type idx)
	{
		if(idx < STATIC_SIZE)
			return m_static_data[idx];
		else if(idx < m_size)
			return m_data[idx - STATIC_SIZE];

		static T dummy{};
		return dummy;
	}


	const value_type& operator[](size_type idx) const
	{
		value_type& val = const_cast<vec_raw<T, STATIC_SIZE>*>(this)->operator[](idx);
		return const_cast<const value_type&>(val);
	}


protected:
	/**
	 * allocate dynamic elements
	 */
	void alloc(size_type num)
	{
		if(!m_data)
		{
			std::cout << "Allocating " << num << " items, this = " << (void*)this << std::endl;

			m_data = reinterpret_cast<T*>(std::malloc(num*sizeof(T)));
		}
		else
		{
			std::cout << "Reallocating " << num << " items at address " << (void*)m_data << ", this = " << (void*)this << std::endl;

			m_data = reinterpret_cast<T*>(std::realloc(m_data, num*sizeof(T)));
		}


		// initialise new elements
		size_type old_dyn_size = (m_size >= STATIC_SIZE ? m_size - STATIC_SIZE : 0);
		for(size_type idx = old_dyn_size; idx < num; ++idx)
			std::construct_at<T>(m_data + idx);

		std::cout << "Initialised dynamic elements " << old_dyn_size << " to " << (num - 1) << std::endl;
	}


private:
	T m_static_data[STATIC_SIZE]{};      // static part of the data
	T* m_data{};                         // dynamic part of the data

	size_type m_size{};                  // number of total elements
	size_type m_reserve{ STATIC_SIZE };  // number of reserved elements
};



int main()
{
	vec_raw<double, 3> vec0 = { 1, 2, 3, 4, 5, 6 };
	vec0.reserve(10);
	vec0.push_back(7);
	vec0.push_back(8);
	vec0.push_back(9);
	vec0.push_back(10);
	vec0.push_back(11);
	std::cout << "vec0 = ";
	for(double d : vec0)
		std::cout << d << " ";
	std::cout << '\n' << std::endl;

	vec_raw<double, 3> vec1 { vec0 };
	std::cout << "vec1 = ";
	for(double d : vec1)
		std::cout << d << " ";
	std::cout << '\n' << std::endl;

	vec_raw<double, 3> vec2 { std::move(vec0) };
	std::cout << "vec2 = ";
	for(double d : vec2)
		std::cout << d << " ";
	std::cout << '\n' << std::endl;

	vec_raw<double, 2> vec3 { std::move(vec1) };
	std::cout << "vec3 = ";
	for(double d : vec3)
		std::cout << d << " ";
	std::cout << '\n' << std::endl;

	vec_raw<double, 2> vec4(5);
	std::cout << "vec4 = ";
	for(double d : vec4)
		std::cout << d << " ";
	std::cout << '\n' << std::endl;

	return 0;
}
