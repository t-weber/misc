/**
 * @author Tobias Weber
 * @date 22-feb-20
 * @license: see 'LICENSE.EUPL' file
 * @see https://www.fernuni-hagen.de/ps/prjs/IROP/
 */

interface IExample
{
		// role management
	void AddRole(String name, IExample role);
	IExample GetRole(String name);

	// an axample basic function
	void func();
};



class ExampleCore implements IExample
{
	@Override public void AddRole(String name, IExample role)
	{
		m_roles.put(name, role);
	}


	@Override public IExample GetRole(String name)
	{
		return m_roles.get(name);
	}


	@Override public void func()
	{
		System.out.println("In " + getClass().getCanonicalName() + "::func()");
	}


	protected java.util.AbstractMap<String, IExample> m_roles = new java.util.HashMap<>();
};



class ExampleRole implements IExample
{

	public ExampleRole(ExampleCore core)
	{
		m_core = core;
	}


	@Override public void AddRole(String name, IExample role)
	{
		m_core.AddRole(name, role);
	}


	@Override public IExample GetRole(String name)
	{
		return m_core.GetRole(name);
	}


	@Override public void func()
	{
		m_core.func();
	}


	// core to forward (or delegate) to
	protected ExampleCore m_core = null;
};



class ConcreteExampleRole1 extends ExampleRole
{
	public ConcreteExampleRole1(ExampleCore core)
	{
		super(core);
	}


	public void func1()
	{
		System.out.println("In " + getClass().getCanonicalName() + "::func1()");
	}
};



class ConcreteExampleRole2 extends ExampleRole
{
	public ConcreteExampleRole2(ExampleCore core)
	{
		super(core);
	}


	public void func2()
	{
		System.out.println("In " + getClass().getCanonicalName() + "::func2()");
	}


	// overriding a core function
	@Override public void func()
	{
		System.out.println("In " + getClass().getCanonicalName() + "::func()");
		func2();
	}
};



class rolobj
{
	public static void main(String[] args)
	{
		// core object
		ExampleCore core = new ExampleCore();


		// example role objects creation
		{
			ConcreteExampleRole1 role1 = new ConcreteExampleRole1(core);
			ConcreteExampleRole2 role2 = new ConcreteExampleRole2(core);
			ExampleRole role3 = new ExampleRole(core)
			{
				@Override public void func()
				{
					System.out.println("In " + getClass().getCanonicalName() + "::func()");
				}
			};

			core.AddRole("role1", role1);
			core.AddRole("role2", role2);
			core.AddRole("role3", role3);
		}


		// example role object usage
		{
			IExample role1 = core.GetRole("role1");
			IExample role2 = core.GetRole("role2");
			IExample role3 = core.GetRole("role3");
			role1.func();
			role2.func();
			role3.func();
		}


		// test
		{
			Class class3 = core.GetRole("role3").getClass();
			System.out.println("\n" + class3 + ": ");
			java.lang.reflect.Method[] methods3 = class3.getMethods();
			java.lang.reflect.Field[] attrs3 = class3.getFields();
			for(java.lang.reflect.Method method : methods3)
				System.out.println("\t" + method);
			for(java.lang.reflect.Field attr : attrs3)
				System.out.println("\t" + attr);
		}
	}
};
