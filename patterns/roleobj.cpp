/**
 * @author Tobias Weber
 * @date 22-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://www.fernuni-hagen.de/ps/prjs/IROP/
 */

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>


class IExample
{
	public:
		// role management
		virtual void AddRole(const std::string& name, std::shared_ptr<IExample> role) = 0;
		virtual const std::shared_ptr<IExample> GetRole(const std::string& name) const = 0;

		// an axample basic function
		virtual void func() = 0;
};



class ExampleCore : public IExample
{
	public:
		virtual void AddRole(const std::string& name, std::shared_ptr<IExample> role) override
		{
			m_roles.insert(std::make_pair(name, role));
		}


		virtual const std::shared_ptr<IExample> GetRole(const std::string& name) const override
		{
			auto iter = m_roles.find(name);
			if(iter == m_roles.end())
				return nullptr;

			return iter->second;
		}


		virtual void func() override
		{
			std::cout << "In ExampleCore::func()" << std::endl;
		}


	protected:
		std::unordered_map<std::string, std::shared_ptr<IExample>> m_roles;
};



class ExampleRole : public IExample
{
	public:
		ExampleRole() = delete;

		ExampleRole(ExampleCore* core) : m_core(core)
		{
		}


		virtual void AddRole(const std::string& name, std::shared_ptr<IExample> role) override
		{
			m_core->AddRole(name, role);
		}


		virtual const std::shared_ptr<IExample> GetRole(const std::string& name) const override
		{
			return m_core->GetRole(name);
		}


		virtual void func() override
		{
			m_core->func();
		}


	protected:
		// core to forward (or delegate) to
		ExampleCore *m_core = nullptr;
};



class ConcreteExampleRole1 : public ExampleRole
{
	public:
		ConcreteExampleRole1() = delete;

		ConcreteExampleRole1(ExampleCore* core) : ExampleRole(core)
		{
		}


		virtual void func1()
		{
			std::cout << "In ConcreteExampleRole1::func1()" << std::endl;
		}
};



class ConcreteExampleRole2 : public ExampleRole
{
	public:
		ConcreteExampleRole2() = delete;

		ConcreteExampleRole2(ExampleCore* core) : ExampleRole(core)
		{
		}


		virtual void func2()
		{
			std::cout << "In ConcreteExampleRole2::func2()" << std::endl;
		}


		// overriding a core function
		virtual void func() override
		{
			std::cout << "In ConcreteExampleRole2::func()" << std::endl;
			func2();
		}
};



int main()
{
	// core object
	auto core = std::make_shared<ExampleCore>();


	// example role objects creation
	{
		auto role1 = std::make_shared<ConcreteExampleRole1>(core.get());
		auto role2 = std::make_shared<ConcreteExampleRole2>(core.get());

		core->AddRole("role1", role1);
		core->AddRole("role2", role2);
	}


	// example role object usage
	{
		auto role1 = core->GetRole("role1");
		auto role2 = core->GetRole("role2");
		role1->func();
		role2->func();
	}

	return 0;
}
