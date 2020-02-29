/**
 * aspect test
 * @author Tobias Weber
 * @date 29-feb-20
 * @license see 'LICENSE.GPL' file
 *
 * @reference https://www.eclipse.org/aspectj/doc/released/progguide/index.html
 *
 * /opt/aspectj/bin/ajc -1.8 -showWeaveInfo -cp .:/opt/aspectj/lib/aspectjrt.jar testaspect.java testaspecttarget.java
 * java -cp .:/opt/aspectj/lib/aspectjrt.jar testaspecttarget
 */

public aspect testaspect
{
	// add a new member function to Test
	@Override public String Test.toString()
	{
		String str = "Total number of function calls: " + this.functioncalls + "\n";
		str += "Memo: ";
		for(Long l : this.memo)
			str += l + " ";

		return str;
	}



	// add a new member variable to Test
	private int Test.recursiondepth = 0;

	// memoisation for recursive function
	//class Memo {
		private boolean Test.usememoisation = true;
		private Long[] Test.memo = null;
		private int Test.functioncalls = 0;
	//}
	//declare parents : Test extends Memo;


	protected pointcut cutRecursiveFunc(int i) :
		call(* *.recursive*(..)) && target(Test) /*&& within(Test)*/ && args(i) /*&& withincode(* recursive*(..))*/;
		//execution(* *.recursive*(..)) && target(Test) && args(i);

	long around(Test t, int i) : cutRecursiveFunc(i) && target(t)
	{
		System.out.println("around(): before joinpoint " + thisJoinPoint + ", arg: " + i
			+ ", recursion depth: " + (t.recursiondepth++));

		// create array for memoisation of results
		if(t.memo == null)
			t.memo = new Long[i+1];

		Long result = t.memo[i];
		if(result == null)
		{
			++t.functioncalls;
			result = proceed(t, i);

			if(t.usememoisation)
				t.memo[i] = result;
		}

		System.out.println("around(): after joinpoint " + thisJoinPoint + ", ret: " + result
			+ ", recursion depth: " + (--t.recursiondepth));

		++totalAdvices;
		return result;
	}



	// global variable
	private int totalAdvices = 0;

	pointcut cutMain() :
		execution(static * main(..));

	/*before*/ after() returning() : cutMain()
	{
		System.out.println();
		System.out.println("after(): " + thisJoinPoint);
		System.out.println("total advices: " + totalAdvices);
	}
};
