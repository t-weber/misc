/**
 * static visitor pattern test
 * @author Tobias Weber
 * @date 7-jun-22
 * @license: see 'LICENSE.EUPL' file
 *
 * @see https://en.wikipedia.org/wiki/Visitor_pattern
 * @see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
 */

#include <memory>
#include <iostream>


class B1;
class B2;


class Visitor
{
public:
	virtual void visit(const B1*) const = 0;
	virtual void visit(const B2*) const = 0;

	virtual ~Visitor() {}
};



template<class t_visitor = Visitor>
class Base
{
public:
	virtual void accept(const t_visitor* visitor) = 0;

	virtual ~Base() {}
};



template<class t_sub, class t_visitor = Visitor>
requires requires(const t_visitor* visitor)
{
	visitor->visit((t_sub*)0);
}
class AcceptVisitor : public Base<t_visitor>
{
public:
	virtual void accept(const t_visitor* visitor) override
	{
		t_sub *_this = static_cast<t_sub*>(this);
		visitor->visit(_this);
	}
};



class B1 : public AcceptVisitor<B1, Visitor>
{
};


class B2 : public AcceptVisitor<B2, Visitor>
{
};



class V1 : public Visitor
{
public:
	virtual void visit(const B1*) const override
	{
		std::cout << "visited B1 with V1" << std::endl;
	}

	virtual void visit(const B2*) const override
	{
		std::cout << "visited B2 with V1" << std::endl;
	}
};


class V2 : public Visitor
{
public:
	virtual void visit(const B1*) const override
	{
		std::cout << "visited B1 with V2" << std::endl;
	}

	virtual void visit(const B2*) const override
	{
		std::cout << "visited B2 with V2" << std::endl;
	}
};



int main()
{
	const std::shared_ptr<Base<Visitor>> bs[] = {
		std::make_shared<B1>(),
		std::make_shared<B2>()
	};
	const std::shared_ptr<Visitor> vs[] = {
		std::make_shared<V1>(),
		std::make_shared<V2>()
	};

	for(const auto& b : bs)
		for(const auto& v : vs)
			b->accept(v.get());

	return 0;
}
