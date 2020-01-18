/**
 * visitor pattern test (simulating double-dispatch with single-dispatch (in C++: virtual functions))
 * @author Tobias Weber
 * @date 18-jan-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Visitor_pattern
 */

#include <iostream>


class B1;
class B2;


class Visitor
{
public:
	virtual void visit(const B1*) const = 0;
	virtual void visit(const B2*) const = 0;
};



#define VISITOR_ACCEPT virtual void accept(const Visitor* visitor) const override { visitor->visit(this); }



class Base
{
public:
	virtual void accept(const Visitor* visitor) const = 0;
};


class B1 : public Base
{
public:
	VISITOR_ACCEPT
};


class B2 : public Base
{
public:
	VISITOR_ACCEPT
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
	const Base* bs[] = { new B1(), new B2() };
	const Visitor* vs[] = { new V1(), new V2() };

	for(auto* b : bs)
		for(auto* v : vs)
			b->accept(v);

	delete bs[0]; delete bs[1];
	delete vs[0]; delete vs[1];

	return 0;
}
