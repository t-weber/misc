/**
 * swig interface test
 * @author Tobias Weber
 * @date 25-mar-20
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *	* http://www.swig.org/tutorial.html
 */

public class test_expr_parser
{
	public static void main(String[] args)
	{
		String cwd = new java.io.File("").getAbsoluteFile().toString();
		//System.loadLibrary("expr_parser");
		System.load(cwd + "/expr_parser.so");

		while(true)
		{
			ExprParserD parser = new ExprParserD();

			System.out.print("Enter an expression: ");
			String line = System.console().readLine();

			Double result = parser.parse(line);
			System.out.println("Result: " + result);
		}
	}
}
