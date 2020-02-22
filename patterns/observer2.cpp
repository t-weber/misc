/**
 * @author Tobias Weber
 * @date 22-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Observer_pattern
 */

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <iostream>


class AbstrObservedRole
{
	public:
		virtual void AddObserver(std::function<void()> o)
		{
			m_observers.push_back(o);
		}


		virtual void NotifyObservers()
		{
			for(const auto& o : m_observers)
				o();
		}

		virtual int GetVar() const = 0;


	protected:
		std::vector<std::function<void()>> m_observers;
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



class Observer
{
	public:
		Observer(const std::string& name) : m_name(name) {}


		void update(std::shared_ptr<AbstrObservedRole> observed)
		{
			std::cout << m_name << ": Observed varable XYZ changed to "
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
	observed->AddObserver([observer1, observed]() { observer1->update(observed); });
	observed->AddObserver([observer2, observed]() { observer2->update(observed); });

	observed->SetVar(123);
	observed->SetVar(567);

	return 0;
}
