/**
 * @author Tobias Weber
 * @date 16-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Visitor_pattern
 */

#include <iostream>


class Tst
{
	public:
		 Tst()
		{
			std::cout << __PRETTY_FUNCTION__ << std::endl;
		}


		void tst()
		{
			std::cout << __PRETTY_FUNCTION__ << std::endl;
			m_inner.tst();
		};


	protected:
		class Inner
		{
			public:
				Inner(Tst* outerThis) : m_outerThis{outerThis}
				{
					std::cout << __PRETTY_FUNCTION__ << std::endl;

					std::cout << "inner this pointer = " << std::hex << (void*)this << std::endl;
					std::cout << "outer this pointer = " << std::hex << (void*)m_outerThis << std::endl;
				}


				void tst()
				{
					std::cout << __PRETTY_FUNCTION__ << std::endl;

					std::cout << "inner this pointer = " << std::hex << (void*)this << std::endl;
					std::cout << "outer this pointer = " << std::hex << (void*)m_outerThis << std::endl;

					int* pInner = &this->m_i;
					int* pOuter = &m_outerThis->m_i;

					std::cout << "inner member pointer = " << std::hex << pInner << std::endl;
					std::cout << "outer member pointer = " << std::hex << pOuter << std::endl;

					std::cout << "inner member value = " << std::dec << *pInner << std::endl;
					std::cout << "outer member value = " << std::dec << *pOuter << std::endl;
				}


			protected:
				int m_i = 987;
				Tst *m_outerThis = nullptr;
		};

		Inner m_inner{this};
		int m_i = 123;
};



int main()
{
	Tst *tst = new Tst();
	tst->tst();

	delete tst;
}
