/**
 * unit tests
 * @author Tobias Weber
 * @date 27-feb-2020
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++17 -o test test.cpp
 * ./test --log_level=all
 *
 * References:
 *	* https://www.boost.org/doc/libs/1_72_0/libs/test/doc/html/index.html
 */

#define TEST_EXAMPLE 1
//#define BOOST_TEST_DYN_LINK


#if TEST_EXAMPLE == 1
#define BOOST_TEST_MODULE Test 1
#endif


#include <tuple>
#include <boost/test/included/unit_test.hpp>
namespace test = boost::unit_test;
namespace testtools = boost::test_tools;


#include <boost/type_index.hpp>
namespace ty = boost::typeindex;


// automatic test cases and suites
#if TEST_EXAMPLE == 1


// a fixture
class Context
{
	public:
		Context() { std::cout << "creating fixture" << std::endl; }
		~Context() { std::cout << "removing fixture" << std::endl; }
};


// optional test suite declaration
// one fixture per suite
BOOST_AUTO_TEST_SUITE(suite_1, *test::fixture<Context>())
// one fixture per test
//BOOST_FIXTURE_TEST_SUITE(suite_1, Context)


// simple test
BOOST_AUTO_TEST_CASE(test_1a)
{
	std::cout << "In " << __func__ << std::endl;

	//BOOST_TEST((2*2 == 5));
	BOOST_TEST(2*2 == 4, "failure in test 1a");
}


// test a template function
template<class T> constexpr T square(T x) { return x*x; }

//using t_types = std::tuple<float, double, long double>;
//BOOST_AUTO_TEST_CASE_TEMPLATE(test_1b, T, t_types)
BOOST_AUTO_TEST_CASE_TEMPLATE(test_1b, T, decltype(std::tuple<float, double, long double>{}))
{
	std::cout << "testing with type: " << ty::type_id_with_cvr<T>().pretty_name() << std::endl; 
	//BOOST_TEST(square<T>(2) == T(4), testtools::tolerance(1e-4));
	BOOST_TEST(square<T>(2) == T(4), testtools::tolerance(std::numeric_limits<T>::epsilon()));
}


// with context (fixture)
BOOST_FIXTURE_TEST_CASE(fixture_test, Context)
{
	BOOST_TEST((4*4 == 16));
}


// optional test suite declaration
BOOST_AUTO_TEST_SUITE_END()



// manual test cases and suites
#elif TEST_EXAMPLE == 2


// test suite
test::test_suite* init_unit_test_suite(int argc, char** argv)
{
	std::cout << "In " << __func__ << std::endl;

	auto* suite = BOOST_TEST_SUITE("Suite 2");
	suite->add(BOOST_TEST_CASE_NAME(
		[]()
		{
			std::cout << "In " << __func__ << std::endl;
			BOOST_CHECK_EQUAL(3*3, 9);
		}, "Test 2a"));

	suite->add(BOOST_TEST_CASE_NAME(
		[]()
		{
			BOOST_TEST(1.1 == 1., testtools::tolerance(0.15));
		}, "Test 2b"));


	//return suite;

	auto& master_suite = test::framework::master_test_suite();
	master_suite.add(suite);
	return nullptr;
}

#endif
