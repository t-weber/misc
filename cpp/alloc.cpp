/**
 * allocator test
 * @author Tobias Weber
 * @date aug-2026
 * @license: see 'LICENSE.EUPL' file
 */

// clang++ -Wall -Wextra -Weffc++ -std=c++20 -o alloc alloc.cpp


#include <vector>
#include <iostream>


/**
 * allocator that doesn't initialise or construct its elements
 * @see https://en.cppreference.com/cpp/memory/allocator
 */
template<class t_elem>
struct alloc_noinit
{
	static constexpr const bool init_elem = false;  // initialise elements?

	using value_type = t_elem;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using propagate_on_container_move_assignment = std::true_type;
	using is_always_equal = std::true_type;


	constexpr alloc_noinit() noexcept = default;
	constexpr alloc_noinit(const alloc_noinit<t_elem>&) noexcept = default;
	//template<class t_other_elem>
	//constexpr alloc_noinit(const alloc_noinit<t_other_elem>&) noexcept = default;

	constexpr ~alloc_noinit() = default;


	constexpr t_elem* allocate(std::size_t num)
	{
		if(num == 0)
			return nullptr;

		//t_elem *t = new t_elem[num];
		//t_elem *t = reinterpret_cast<t_elem*>(new char[sizeof(t_elem)*num]);
		t_elem *t = reinterpret_cast<t_elem*>(std::malloc(sizeof(t_elem)*num));

		std::cout << "Allocated " << std::dec << num << " elements at address "
			<< std::hex << reinterpret_cast<void*>(t) << "." << std::endl;

		return t;
	}


	void deallocate(t_elem *mem, std::size_t num)
	{
		if(!mem)
			return;

		std::cout << "Deallocating " << std::dec << num << " elements at address "
			<< std::hex << reinterpret_cast<void*>(mem) << "." << std::endl;

		//delete[] reinterpret_cast<char*>(mem);
		std::free(reinterpret_cast<void*>(mem));
	}


	template<class t_other_elem, class ...t_args>
	void construct(t_other_elem *mem, t_args&& ...args)
	{
		if constexpr(init_elem)
		{
			if(!mem)
				return;

			std::cout << "Constructing element at address "
				<< std::hex << reinterpret_cast<void*>(mem) << "." << std::endl;

			// call constructor
			void *pmem = reinterpret_cast<void*>(mem);
			new(pmem) t_other_elem{ std::forward<t_args>(args)... };
		}
	}


	template<class t_other_elem>
	void destroy(t_other_elem *t)
	{
		if constexpr(init_elem)
		{
			if(!t)
				return;

			std::cout << "Destroying element at address "
				<< std::hex << reinterpret_cast<void*>(t) << "." << std::endl;

			// call destructor
			t->~t_other_elem();
		}
	}
};



struct A
{
	A()
	{
		std::cout << "A::A(), this = "
			<< reinterpret_cast<void*>(this)
			<< "." << std::endl;
	}

	A(const A&)
	{
		std::cout << "A::A() copy, this = "
			<< reinterpret_cast<void*>(this)
			<< "." << std::endl;
	}

	A(A&&)
	{
		std::cout << "A::A() move, this = "
			<< reinterpret_cast<void*>(this)
			<< "." << std::endl;
	}

	~A()
	{
		std::cout << "A::~A(), this = "
			<< reinterpret_cast<void*>(this)
			<< "." << std::endl;
	}

	A& operator=(const A&)
	{
		std::cout << "A copy, this = "
			<< reinterpret_cast<void*>(this)
			<< "." << std::endl;

		return *this;
	}

	A& operator=(A&&)
	{
		std::cout << "A move, this = "
			<< reinterpret_cast<void*>(this)
			<< "." << std::endl;

		return *this;
	}
};



int main()
{
	{
		std::cout << "vector<int> definition" << std::endl;
		std::vector<int, alloc_noinit<int>> vec(3);
		std::cout << "vec = " << std::dec << vec[0] << " " << vec[1] << " " << vec[2] << std::endl;

		std::cout << "\nvector<int> element assignments" << std::endl;
		vec[0] = 1;
		vec[1] = 2;
		vec[2] = 3;
		std::cout << "vec = " << std::dec << vec[0] << " " << vec[1] << " " << vec[2] << std::endl;
	}

	std::cout << "\n--------------------------------------------------------------------------------" << std::endl;

	{
		std::cout << "\nvector<A> definition" << std::endl;
		std::vector<A, alloc_noinit<A>> vec2(3);

		std::cout << "\nvector<A> element assignments" << std::endl;
		vec2[0] = A{};
		vec2[1] = A{};
		vec2[2] = A{};
	}

	return 0;
}
