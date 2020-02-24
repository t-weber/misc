/**
 * @author Tobias Weber
 * @date 22-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Observer_pattern
 */

#include <vector>
#include <memory>
#include <string>
#include <iostream>


class AbstrObservedRole;


class IObserverRole
{
	public:
		virtual void update(const AbstrObservedRole*) = 0;
};


class AbstrObservedRole
{
	public:
		virtual void AddObserver(std::shared_ptr<IObserverRole> o)
		{
			m_observers.push_back(o);
		}


		virtual void NotifyObservers()
		{
			for(auto o : m_observers)
				o->update(this);
		}

		virtual int GetVar() const = 0;


	protected:
		std::vector<std::shared_ptr<IObserverRole>> m_observers;
};



class Observed : public AbstrObservedRole
{
	public:
		void SetVar(int var)
		{
			this->m_var = var;
			NotifyObservers();
		}

		int GetVar() const
		{
			return m_var;
		}

	protected:
		int m_var = 0;
};


class Observer : public IObserverRole
{
	public:
		Observer(const std::string& name) : m_name(name) {}


		virtual void update(const AbstrObservedRole* observed) override
		{
			std::cout << m_name << ": Observed variable change to "
				<< observed->GetVar() << "." << std::endl;
		}


	protected:
		std::string m_name = "Observer";
};


int main()
{
	auto observed = std::make_shared<Observed>();
	auto observer1 = std::make_shared<Observer>("Observer 1");
	auto observer2 = std::make_shared<Observer>("Observer 2");
	observed->AddObserver(observer1);
	observed->AddObserver(observer2);

	observed->SetVar(123);
	observed->SetVar(567);

	return 0;
}
