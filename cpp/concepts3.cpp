/**
 * concepts test (using static and dynamic constructors)
 * @author Tobias Weber
 * @date dec-17
 * @license: see 'LICENSE' file
 *
 * enabled concepts:
 * g++ -o concepts3 concepts3.cpp -std=c++17 -fconcepts
 */

#include <iostream>
#include <array>
#include <vector>


// requirements of a vector type (as concept variable)
template<class T>
concept bool is_vec = requires(const T& a)
{
	typename T::value_type;		// must have a value_type
	a.operator[](1);		// must have operator[]
	{ a.size() } -> std::size_t;	// must have a size() member function
};

// requirements of a vector type (as concept function)
template<class T>
concept bool is_vec_func()
{
	return requires(const T& a)
	{
		typename T::value_type;				// must have a value_type
		{ a.operator[](1) } -> typename T::value_type;	// must have operator[]
		{ a.size() } -> std::size_t;			// must have a size() member function
	};
}


// requirements of a vector type with a dynamic size
template<class T>
concept bool is_dyn_vec = requires(const T& a)
{
	T(3);						// constructor
};



// a function on the vector type
template<class t_vec>
t_vec vector_func(const t_vec& vec1, const t_vec& vec2)
	requires is_vec<t_vec>			// using variable concept
	//requires is_vec_func<t_vec>()		// using function concept
{
	t_vec vec;
	if constexpr(is_dyn_vec<t_vec>)
	{
		vec = t_vec(vec1.size());
		std::cout << "Vector dynamically constructed on heap.\n";
	}
	else
	{
		std::cout << "Vector statically constructed on stack.\n";
	}

	for(std::size_t i=0; i<vec1.size(); ++i)
		vec[i] = vec1[i] + vec2[2];

	return vec;
}


template<class T> concept bool has_size = requires(const T& t) { t.size(); };

int main()
{
	// using static std::array
	std::array<double, 3> vec1, vec2;
	vec1[0] = 1.; vec1[1] = 2.; vec1[2] = 3.;
	vec2[0] = 9.; vec2[1] = 8.; vec2[2] = 7.;
	std::array<double, 3> vecR1 = vector_func(vec1, vec2);
	std::cout << vecR1[0] << ", " << vecR1[1] << ", " << vecR1[2] << "\n";


	// using dynamic std::vector
	std::vector<double> vec3(3), vec4(3);
	vec3[0] = 1.; vec3[1] = 2.; vec3[2] = 3.;
	vec4[0] = 9.; vec4[1] = 8.; vec4[2] = 7.;
	std::vector<double> vecR2 = vector_func(vec3, vec4);
	std::cout << vecR2[0] << ", " << vecR2[1] << ", " << vecR2[2] << "\n";


	// checking if a member is available
	auto lam = [](const auto& vec) -> void
	{
		//concept bool has_size = requires(const auto& t) { t.size(); };

		if constexpr(has_size<decltype(vec)>)
			std::cout << "Type " << typeid(vec).name() << " has a size() member.\n";
		else
			std::cout << "Type " << typeid(vec).name() << " has NO size() member.\n";
	};

	lam(vec1);
	lam(5);


	return 0;
}
