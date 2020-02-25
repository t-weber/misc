/**
 * proxy pattern
 * @author Tobias Weber
 * @date 25-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Proxy_pattern
 */

#include <vector>
#include <memory>
#include <string>
#include <iostream>


class IBase
{
	public:
		virtual void func() = 0;
};


class Subj : public IBase
{
	public:
		virtual void func() override
		{
			std::cout << "Subject::func()" << std::endl;
		}
};


class Proxy : public IBase
{
	public:
		Proxy(std::shared_ptr<IBase> subj) : m_subj{subj}
		{}

		virtual void func() override
		{
			std::cout << "Proxy::func()" << std::endl;

			// forwarding
			m_subj->func();
		}


	protected:
		std::shared_ptr<IBase> m_subj;
};


int main()
{
	auto subj = std::make_shared<Proxy>(std::make_shared<Subj>());
	subj->func();

	return 0;
}
