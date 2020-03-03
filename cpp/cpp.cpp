/**
 * C++ tests
 * @author Tobias Weber
 * @date 3-mar-20
 * @license: see 'LICENSE.EUPL' file
 *
 * clang++ -o cpp cpp.cpp -std=c++17
 */

#include <iostream>
#include <memory>


// ----------------------------------------------------------------------------
// test covariance of return type
// ----------------------------------------------------------------------------

//#define TST_CONTRARET		// return type contravariance ist not allowed
//#define TST_CONTRAARG		// argument type contravariance is not allowed
//#define TST_COVARG		// argument type covariance is not allowed


class BaseType {};
class SubType : public BaseType {};


class X
{
public:
	virtual BaseType* tstCovRet()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return new BaseType{};
	}

	virtual const BaseType& tstCovRefRet()
	{
		static const BaseType _var;
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return _var;
	}

	virtual std::shared_ptr<BaseType> tstCovRetShared()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return std::make_shared<BaseType>();
	}

#ifdef TST_CONTRARET
	virtual SubType* tstContraRet()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return new SubType{};
	}
#endif

#ifdef TST_CONTRAARG
	virtual void tstContraArg(SubType*)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	}
#endif

#ifdef TST_COVARG
	virtual void tstCovArg(BaseType*)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	}
#endif
};


class Y : public X
{
public:
	virtual SubType* tstCovRet() override
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return new SubType{};
	}

	virtual const SubType& tstCovRefRet() override
	{
		static const SubType _var;
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return _var;
	}

	//virtual std::shared_ptr<SubType> tstCovRetShared() override
	virtual std::shared_ptr<BaseType> tstCovRetShared() override
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return std::make_shared<SubType>();
	}

#ifdef TST_CONTRARET
	virtual BaseType* tstContraRet() override
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		return new BaseType{};
	}
#endif

#ifdef TST_CONTRAARG
	virtual void tstContraArg(BaseType*) override
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	}
#endif

#ifdef TST_COVARG
	virtual void tstCovArg(SubType*) override
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
	}
#endif
};


void tstCov()
{
	std::shared_ptr<X> y = std::make_shared<Y>();

	BaseType* b = y->tstCovRet();
	delete b;

	const BaseType& b2 = y->tstCovRefRet();

	std::shared_ptr<BaseType> b3 = y->tstCovRetShared();
}

// ----------------------------------------------------------------------------


int main()
{
	tstCov();
	return 0;
}
