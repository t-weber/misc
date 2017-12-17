/**
 * concepts test
 * @author Tobias Weber
 * @date dec-17
 * @license: see 'LICENSE' file
 *
 * enabled concepts:
 * g++ -o concepts2 concepts2.cpp -std=c++17 -fconcepts
 *
 * disabled concepts:
 * g++ -o concepts2 concepts2.cpp -std=c++17
 */

#include <iostream>
#include <string>
#include <type_traits>
#include <boost/numeric/ublas/vector.hpp>

namespace ublas = boost::numeric::ublas;


#ifdef __cpp_concepts
// requirements of a vector type
template<class T>
concept bool is_vec = requires(const T& a)
{
	typename T::value_type;		// must have a value_type
	T(3);				// constructor
	a.operator[](1);		// must have operator[]
	a.size();			// must have a size() member function
};

// second requirement
template<class T>
concept bool is_vec2 = requires(const T& a)
{
	T();
};
#endif



// a function on the vector type
template<class t_vec>
t_vec vector_func(const t_vec& vec1, const t_vec& vec2)
#ifdef __cpp_concepts
	requires is_vec<t_vec> && is_vec2<t_vec>
#endif
{
	t_vec vec(vec1.size());

	for(std::size_t i=0; i<vec1.size(); ++i)
		vec[i] = vec1[i] + vec2[2];

	return vec;
}


#ifdef __cpp_concepts
// alternate formulation
template<is_vec t_vec>
void vector_func2(const t_vec& vec1, const t_vec& vec2)
{
}
#endif


// test vector
struct t_tstvec
{
	using value_type = double;
	constexpr static std::size_t m_size = 3;

	value_type vec[m_size];

	value_type& operator[](int i) { return vec[i]; };
	const value_type& operator[](int i) const { return vec[i]; };
	std::size_t size() const { return m_size; }

	// dummy constructors
	t_tstvec(std::size_t) {}
	t_tstvec() = default;
	t_tstvec(const t_tstvec&) = default;
};



int main()
{
	// using test vector
	t_tstvec vec1, vec2;
	vec1[0] = 1.; vec1[1] = 2.; vec1[2] = 3.;
	vec2[0] = 9.; vec2[1] = 8.; vec2[2] = 7.;
	t_tstvec vecR1 = vector_func(vec1, vec2);
	std::cout << vecR1[0] << ", " << vecR1[1] << ", " << vecR1[2] << "\n";

	// using boost vector
	ublas::vector<double> vec3(3), vec4(3);
	vec3[0] = 1.; vec3[1] = 2.; vec3[2] = 3.;
	vec4[0] = 9.; vec4[1] = 8.; vec4[2] = 7.;
	ublas::vector<double> vecR2 = vector_func(vec3, vec4);
	std::cout << vecR2[0] << ", " << vecR2[1] << ", " << vecR2[2] << "\n";

	return 0;
}
