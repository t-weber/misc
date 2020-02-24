/**
 * visitor pattern test (simulating double-dispatch with single-dispatch)
 * @author Tobias Weber
 * @date 24-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Visitor_pattern
 */

 
// ----------------------------------------------------------------------------
// interfaces
// ----------------------------------------------------------------------------
interface Visitor
{
	public void visit(B1 b1);
	public void visit(B2 b2);
};


interface Base
{
	public void accept(Visitor visitor);
};
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
class B1 implements Base
{
	@Override
	public void accept(Visitor visitor)
	{
		visitor.visit(this);
	}
};


class B2 implements Base
{
	@Override
	public void accept(Visitor visitor)
	{
		visitor.visit(this);
	}
};



class V1 implements Visitor
{
	@Override
	public void visit(B1 b1)
	{
		System.out.println("visited B1 with V1");
	}

	@Override
	public void visit(B2 b2)
	{
		System.out.println("visited B2 with V1");
	}
};


class V2 implements Visitor
{
	@Override
	public void visit(B1 b1)
	{
		System.out.println("visited B1 with V2");
	}

	@Override
	public void visit(B2 b2)
	{
		System.out.println("visited B2 with V2");
	}
};
// ----------------------------------------------------------------------------




public class visitor
{
	public static void main(String[] args)
	{
		Base[] bs = { new B1(), new B2() };
		Visitor[] vs = { new V1(), new V2() };

		for(Base b : bs)
			for(Visitor v : vs)
				b.accept(v);
	}
}
