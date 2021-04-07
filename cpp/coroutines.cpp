/**
 * coroutine test
 * @author Tobias Weber
 * @date apr-21
 * @license see 'LICENSE.EUPL' file
 *
 * @see https://en.cppreference.com/w/cpp/language/coroutines
 *
 * g++ -std=c++20 -fcoroutines -o coroutines coroutines.cpp
 */

#include <coroutine>
#include <iostream>


// ----------------------------------------------------------------------------
class CoAwait
{
public:
	CoAwait()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	}

	bool await_ready() const
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return false;
	}

	template<class t_promise>
	bool await_suspend(std::coroutine_handle<t_promise>& handler) const
	{
		std::cout << __PRETTY_FUNCTION__ << ", before resuming handler" << std::endl;

		if(handler)
		{
			std::cout << "handler done: " << std::boolalpha << handler.done() << std::endl;

			t_promise& promise = handler.promise();
			promise.SetVal(123);

			// resume after co_await
			handler();
		}

		std::cout << __PRETTY_FUNCTION__ << ", after resuming handler" << std::endl;
		return true;
	}

	void await_resume() const
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	}
};


class CoAwaitFunc
{
public:
	CoAwait operator co_await() const
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return CoAwait{};
	}
};
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
template<class t_retval>
class CoPromise
{
public:
	CoPromise()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	}

	auto initial_suspend() const
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return std::suspend_never{} /*std::suspend_always{}*/;
	}

	auto final_suspend() const
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return std::suspend_never{} /*std::suspend_always{}*/;
	}

	/**
	 * passes the value to the CoReturn constructor
	 */
	auto get_return_object()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return std::coroutine_handle<CoPromise<t_retval>>::from_promise(*this);
	}

	void unhandled_exception() const
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		throw;
	}

	void SetVal(t_retval val) { this->val = val; }
	t_retval GetVal() const { return val; }

private:
	t_retval val{};
};


template<class t_val>
class CoReturn
{
public:
	using promise_type = CoPromise<t_val>;

	CoReturn() = delete;

	/**
	 * constructor copying the value from the promise
	 */
	CoReturn(const std::coroutine_handle<promise_type>& handle)
		: val{handle.promise().GetVal()}
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	}

	t_val GetVal() const { return val; }

private:
	t_val val{};
};
// ----------------------------------------------------------------------------


/**
 * coroutine
 */
template<class t_retval>
CoReturn<t_retval> start_coroutine()
{
	std::cout << __PRETTY_FUNCTION__ << ", before co_await" << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	co_await CoAwaitFunc{};
	//co_await CoAwait{};

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << __PRETTY_FUNCTION__ << ", after co_await" << std::endl;
}


int main()
{
	try
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;

		auto ret = start_coroutine<int>();
		auto retval = ret.GetVal();

		std::cout << __PRETTY_FUNCTION__
			<< ", return value: " << retval
			<< " of type " << typeid(decltype(retval)).name()
			<< std::endl;
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return 0;
}
