/**
 * composite pattern
 * @author Tobias Weber
 * @date 23-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Composite_pattern
 */

#include <vector>
#include <memory>
#include <string>
#include <iostream>


// ----------------------------------------------------------------------------
// variant 1
// ----------------------------------------------------------------------------
class IComponent
{
	public:
		virtual void AddChild(std::shared_ptr<IComponent> child) = 0;
		virtual std::size_t ChildCount() = 0;
		virtual std::shared_ptr<IComponent> GetChild(std::size_t idx) = 0;
		virtual bool IsLeaf() const = 0;

		// example function
		virtual void func() = 0;
};


class Composite : public IComponent
{
	public:
		virtual void AddChild(std::shared_ptr<IComponent> child) override
		{
			m_children.push_back(child);
		}

		virtual std::size_t ChildCount() override
		{
			return m_children.size();
		}

		virtual std::shared_ptr<IComponent> GetChild(std::size_t idx) override
		{
			return m_children.at(idx);
		}

		virtual bool IsLeaf() const override { return 0; }


		virtual void func() override
		{
			std::cout << "in composite" << std::endl;
		}


	protected:
		std::vector<std::shared_ptr<IComponent>> m_children;
};


class Leaf : public IComponent
{
	public:
		virtual void AddChild(std::shared_ptr<IComponent> child) override
		{
			throw std::runtime_error("Leaf has no child nodes.");
		}

		virtual std::size_t ChildCount() override
		{
			throw std::runtime_error("Leaf has no child nodes.");
		}

		virtual std::shared_ptr<IComponent> GetChild(std::size_t idx) override
		{
			throw std::runtime_error("Leaf has no child nodes.");
		}

		virtual bool IsLeaf() const override { return 1; }


		virtual void func() override
		{
			std::cout << "in leaf" << std::endl;
		}
};
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// variant 2
// ----------------------------------------------------------------------------
class IComponentVar2
{
	public:
		virtual bool IsLeaf() const = 0;

		// example function
		virtual void func() = 0;
};


class IComponentsVar2
{
	public:
		virtual void AddChild(std::shared_ptr<IComponentVar2> child) = 0;
		virtual std::size_t ChildCount() = 0;
		virtual std::shared_ptr<IComponentVar2> GetChild(std::size_t idx) = 0;


		//virtual bool IsLeaf() const = 0;

		// example function
		//virtual void func() = 0;
};


class CompositeVar2 : public IComponentVar2, public IComponentsVar2
{
	public:
		virtual void AddChild(std::shared_ptr<IComponentVar2> child) override
		{
			m_children.push_back(child);
		}

		virtual std::size_t ChildCount() override
		{
			return m_children.size();
		}

		virtual std::shared_ptr<IComponentVar2> GetChild(std::size_t idx) override
		{
			return m_children.at(idx);
		}

		virtual bool IsLeaf() const override { return 0; }


		virtual void func() override
		{
			std::cout << "in composite" << std::endl;
		}


	protected:
		std::vector<std::shared_ptr<IComponentVar2>> m_children;
};


class LeafVar2 : public IComponentVar2
{
	public:
		virtual bool IsLeaf() const override { return 1; }

		virtual void func() override
		{
			std::cout << "in leaf" << std::endl;
		}
};

// ----------------------------------------------------------------------------



int main()
{
	try
	{
		std::shared_ptr<IComponent> root = std::make_shared<Composite>();
		root->AddChild(std::make_shared<Leaf>());
		root->AddChild(std::make_shared<Leaf>());

		std::cout << root->ChildCount() << std::endl;
		std::cout << root->IsLeaf() << " " << root->GetChild(0)->IsLeaf() << std::endl;
		root->func();
		root->GetChild(0)->func();
		root->GetChild(1)->func();

		std::shared_ptr<IComponent> tst = std::make_shared<Leaf>();
		tst->ChildCount();
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}


	std::cout << std::endl;

	try
	{
		auto root = std::make_shared<CompositeVar2>();
		// root has two roles:
		std::shared_ptr<IComponentsVar2> rootAsComps = std::dynamic_pointer_cast<IComponentsVar2>(root);
		std::shared_ptr<IComponentVar2> rootAsComp = std::dynamic_pointer_cast<IComponentVar2>(root);
		//IComponentsVar2* rootAsComps = dynamic_cast<IComponentsVar2*>(root.get());
		//IComponentVar2* rootAsComp = dynamic_cast<IComponentVar2*>(root.get());

		rootAsComps->AddChild(std::make_shared<LeafVar2>());
		rootAsComps->AddChild(std::make_shared<LeafVar2>());
		std::cout << rootAsComps->ChildCount() << std::endl;
		std::cout << rootAsComp->IsLeaf() << " " << root->GetChild(0)->IsLeaf() << std::endl;
		rootAsComp->func();
		rootAsComps->GetChild(0)->func();
		rootAsComps->GetChild(1)->func();
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return 0;
}
