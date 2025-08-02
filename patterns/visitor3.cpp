/**
 * visitor pattern test via std::visit
 * @author Tobias Weber
 * @date 2-aug-25
 * @license: see 'LICENSE.EUPL' file
 */

#include <variant>
#include <array>
#include <memory>
#include <print>


struct B1
{
};


struct B2
{
};


struct Visitor
{
	virtual void operator()(const B1& b1) const = 0;
	virtual void operator()(const B2& b2) const = 0;

	virtual ~Visitor() = default;
};


struct Visitor1 : Visitor
{
	void operator()(const B1&) const
	{
		std::println("visited B1 with V1.");
	}


	void operator()(const B2&) const
	{
		std::println("visited B2 with V1.");
	}
};


struct Visitor2 : Visitor
{
	void operator()(const B1&) const
	{
		std::println("visited B1 with V2.");
	}


	void operator()(const B2&) const
	{
		std::println("visited B2 with V2.");
	}
};


int main()
{
	using t_var = std::variant<B1, B2>;
	std::array<t_var, 2> vars
	{{
		B1{},
		B2{}
	}};

	std::array<std::shared_ptr<Visitor>, 2> visitors
	{{
		std::make_shared<Visitor1>(),
		std::make_shared<Visitor2>()
	}};

	// visit all vars with all visitors
	for(std::shared_ptr<Visitor> visitor : visitors)
		for(const t_var& var : vars)
			std::visit(*visitor, var);

	return 0;
}
