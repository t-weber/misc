/**
 * proxy pattern
 * @author Tobias Weber
 * @date 25-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Proxy_pattern
 */


interface IBase
{
	public void func();
};


class Subj implements IBase
{
	@Override
	public  void func()
	{
		System.out.println("Subject::func()");
	}
};


class Proxy implements IBase
{
	public Proxy(IBase subj)
	{
		this.m_subj = subj;
	}

	@Override
	public void func()
	{
		System.out.println("Proxy::func()");

		// forwarding
		m_subj.func();
	}


	protected IBase m_subj = null;
};


public class proxy
{
	public static void main(String[] args)
	{
		IBase subj = new Proxy(new Subj());
		subj.func();
	}
}

