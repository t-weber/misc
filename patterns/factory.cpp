/**
 * factory method pattern
 * @author Tobias Weber
 * @date 24-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Factory_method_pattern
 */

#include <vector>
#include <memory>
#include <string>
#include <iostream>


// ----------------------------------------------------------------------------
// interfaces and abstract classes
// ----------------------------------------------------------------------------
class IProduct
{
	public:
		virtual void func() = 0;
};


class AbstractFactory
{
	public:
		// factory method
		virtual std::shared_ptr<IProduct> create() = 0;

		std::shared_ptr<IProduct> GetProd()
		{
			return create();
		}
};
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
template<char name>
class Product : public IProduct
{
	public:
		virtual void func() override
		{
			std::cout << "Product " << name << std::endl;
		}
};

using ProductA = Product<'A'>;
using ProductB = Product<'B'>;


template<class t_product>
class Factory : public AbstractFactory
{
	public:
		virtual std::shared_ptr<IProduct> create() override
		{
			return std::make_shared<t_product>();
		}
};

using FactoryA = Factory<ProductA>;
using FactoryB = Factory<ProductB>;
// ----------------------------------------------------------------------------



int main()
{
	FactoryA a;
	a.GetProd()->func();

	FactoryB b;
	b.GetProd()->func();

	return 0;
}
