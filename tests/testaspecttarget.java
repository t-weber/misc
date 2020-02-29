/**
 * aspect test
 * @author Tobias Weber
 * @date 29-feb-20
 * @license: see 'LICENSE.GPL' file
 */

class Test
{
	public long recursiveFunc(int i)
	{
		if(i==0 || i==1) return 1;
		return recursiveFunc(i-1) + recursiveFunc(i-2);
	}
};


public class testaspecttarget
{
	public static void main(String[] args)
	{
		Test tst = new Test();
		long result = tst.recursiveFunc(10);
		System.out.println("Result: " + result);
		System.out.println("Test class: " + tst.toString());


		System.out.println();
		long result2 = tst.recursiveFunc(5);
		System.out.println("Result 2: " + result2);
		System.out.println("Test class: " + tst.toString());


		System.out.println();
		Test tst3 = new Test();
		long result3 = tst3.recursiveFunc(5);
		System.out.println("Result 3: " + result3);
		System.out.println("Test class 3: " + tst3.toString());
	}
};

