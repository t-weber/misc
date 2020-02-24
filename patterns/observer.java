/**
 * observer pattern
 * @author Tobias Weber
 * @date 24-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Observer_pattern
 */

// ----------------------------------------------------------------------------
interface IObserverRole
{
	void update(AbstrObservedRole o);
};


abstract class AbstrObservedRole
{
	public void AddObserver(IObserverRole o)
	{
		m_observers.add(o);
	}


	public void NotifyObservers()
	{
		for(IObserverRole o : m_observers)
			o.update(this);
	}

	public abstract int GetVar();


	protected java.util.List<IObserverRole> m_observers = new java.util.Vector<>();
};
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
class Observed extends AbstrObservedRole
{
	public void SetVar(int var)
	{
		this.m_var = var;
		NotifyObservers();
	}

	public int GetVar()
	{
		return m_var;
	}

	protected int m_var = 0;
};


class Observer implements IObserverRole
{
	public Observer(String name)
	{
		m_name = name;
	}

	@Override
	public void update(AbstrObservedRole observed)
	{
		System.out.println(m_name + ": Observed variable change to " + observed.GetVar() + ".");
	}


	protected String m_name = "Observer";
};
// ----------------------------------------------------------------------------



public class observer
{
	public static void main(String[] args)
	{
		Observed observed = new Observed();
		IObserverRole observer1 = new Observer("Observer 1");
		IObserverRole observer2 = new Observer("Observer 2");
		observed.AddObserver(observer1);
		observed.AddObserver(observer2);

		observed.SetVar(123);
		observed.SetVar(567);
	}
}
