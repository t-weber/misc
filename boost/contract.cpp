/**
 * contract tests
 * @author Tobias Weber
 * @date 8-dec-19
 * @license: see 'LICENSE.EUPL' file
 *
 * g++ -std=c++17 -o contract contract.cpp
 *
 * References:
 *	* https://www.boost.org/doc/libs/1_71_0/libs/contract/doc/html/index.html
 *	* https://www.boost.org/doc/libs/1_71_0/libs/contract/doc/html/boost_contract/tutorial.html
 */

#include <iostream>
#include <cmath>
#include <experimental/source_location>

#include <boost/contract.hpp>
namespace contr = boost::contract;



template<class t_real = float>
t_real func1(t_real f)
{
	std::cout << "Entering " << __func__ << "..." << std::endl;

	// original value of f at time of function call
	auto _fprime = BOOST_CONTRACT_OLDOF(f);

	// pre- and post-conditions
	auto _precondition = [&]() -> void
	{
		std::cout << "Checking pre-condition... " << std::endl;
		BOOST_CONTRACT_ASSERT((*_fprime >= t_real{0}));
	};

	auto _postcondition = [&]() -> void
	{
		std::cout << "Checking post-condition... " << std::endl;
		std::cout << *_fprime << " -> " << f << std::endl;
		BOOST_CONTRACT_ASSERT((!std::isnan(f)));
		BOOST_CONTRACT_ASSERT(std::abs(f - std::sqrt(*_fprime)) < 0.0001);
	};

	contr::check _check{contr::function().precondition(_precondition).postcondition(_postcondition)};


	f = std::sqrt(f);
	std::cout << "Exiting " << __func__ << "..." << std::endl;
	return f;
}



class A : private contr::constructor_precondition<A>
{
	public:
		A() : contr::constructor_precondition<A>([&]() -> void
		{
			std::cout << "Checking constructor pre-condition ..." << std::endl;
			//BOOST_CONTRACT_ASSERT(c == 123);
		})
		{}


		virtual ~A()
		{
			auto _postcondition = [&]() -> void
			{
				std::cout << "Checking destructor post-condition ..." << std::endl;
				BOOST_CONTRACT_ASSERT(c == 123);
			};

			contr::check _check{contr::destructor(this).postcondition(_postcondition)};
		}


		void invariant() const
		{
			std::cout << "Checking invariant..." << std::endl;
			BOOST_CONTRACT_ASSERT(c == 123);
		}


		int f(int i)
		{
			std::cout << "Entering " << __func__ << "..." << std::endl;

			auto _iprime = BOOST_CONTRACT_OLDOF(i);

			auto _precondition = [&]() -> void
			{
				std::cout << "Checking member function pre-condition... " << std::endl;
				BOOST_CONTRACT_ASSERT(*_iprime > 0);
			};

			auto _postcondition = [&]() -> void
			{
				std::cout << "Checking member function post-condition... " << std::endl;
				BOOST_CONTRACT_ASSERT(i == *_iprime * *_iprime);
			};


			// pre- and post-conditions
			contr::check _check{contr::public_function(this)
				.precondition(_precondition).postcondition(_postcondition)};

			i = i*i;

			std::cout << "Exiting " << __func__ << "..." << std::endl;
			return i;
		}


	private:
		const int c = 123;
};



int main()
{
	// error handlers
	auto err = [](const auto& whence) -> void
	{
		auto loc = std::experimental::source_location::current();
		std::cerr << "Condition failed in "
			<< loc.function_name() << ", l. " << loc.line() << " (" << loc.file_name() << ")."
			<< " Whence = " << whence << "."
			<< std::endl;
		throw;
	};

	contr::set_precondition_failure(err);
	contr::set_postcondition_failure(err);
	contr::set_invariant_failure(err);



	try
	{
		std::cout << "Testing function..." << std::endl;
		func1(123.f);
		func1(-1.f);
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}



	try
	{
		std::cout << "\nTesting member function..." << std::endl;
		A a{};
		a.f(987);
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}



	try
	{
		std::cout << "\nTesting lambda function..." << std::endl;
		auto func2 = [](int i) -> void
		{
			std::cout << "Entering " << __func__ << "..." << std::endl;

			auto _iprime = BOOST_CONTRACT_OLDOF(i);

			auto _precondition = [i]() -> void
			{
				std::cout << "Checking pre-condition... " << std::endl;

				if(i < 0)
					throw std::out_of_range("Condition: i >= 0");
			};

			auto _postcondition = [&i, &_iprime]() -> void
			{
				std::cout << "Checking post-condition... " << std::endl;

				if(i != *_iprime)
					throw std::out_of_range("Condition: i == i'*i'");
			};

			contr::check _check{contr::function()
				.precondition(_precondition).postcondition(_postcondition)};

			std::cout << "Exiting " << __func__ << "..." << std::endl;
		};

		func2(12);
		func2(-12);
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return 0;
}
