/**
 * factory method pattern
 * @author Tobias Weber
 * @date 24-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Factory_method_pattern
 */

// ----------------------------------------------------------------------------
// interfaces and abstract classes
// ----------------------------------------------------------------------------
interface IProduct
{
	void func();
};


abstract class AbstractFactory
{
	// factory method
	public abstract IProduct create();

	public IProduct GetProd()
	{
		return create();
	}
};
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
class ProductA implements IProduct
{
	@Override public void func()
	{
		System.out.println("Product A");
	}
};


class ProductB implements IProduct
{
	@Override public void func()
	{
		System.out.println("Product B");
	}
};


class FactoryA extends AbstractFactory
{
	@Override public IProduct create()
	{
		return new ProductA();
	}
};


class FactoryB extends AbstractFactory
{
	@Override public IProduct create()
	{
		return new ProductB();
	}
};



// ----------------------------------------------------------------------------



public class factory
{
	public static void main(String[] args)
	{
		AbstractFactory a = new FactoryA();
		IProduct proda = a.GetProd();

		AbstractFactory b = new FactoryB();
		IProduct prodb = b.GetProd();

		proda.func();
		prodb.func();
	}
}

