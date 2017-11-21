/**
 * command line args tests
 * @author Tobias Weber
 * @date 19-nov-17
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/doc/html/program_options.html
 *  * https://github.com/boostorg/program_options/tree/develop/example
 *
 * gcc -o args args.cpp -std=c++17 -lboost_program_options -lstdc++
 */

#include <iostream>
#include <list>
#include <string>

#include <boost/program_options.hpp>
namespace args = boost::program_options;


int main(int argc, char** argv)
{
	std::vector<std::string> vecPosArgs, vecUnreg;

	// describe args
	// main group, with positional args
	int a = 0;
	double b = 0.;
	bool bSwitch = 0;
	args::options_description arg_descr("Arg group 1");
	arg_descr.add_options()
		(",a", args::value(&a), "a value")
		(",b", args::value(&b), "b value")
		("switch,s", args::bool_switch(&bSwitch), "bool switch")
		("posarg", args::value<decltype(vecPosArgs)>(&vecPosArgs), "positional args");


	// second arg group, with default and implicit values
	int c = 0, d = 0;
	{
		args::options_description arg_descr2("Arg group 2");
		arg_descr2.add_options()
			(",c", args::value(&c)->default_value(123), "c value")
			(",d", args::value(&d)->implicit_value(456), "d value");
		arg_descr.add(arg_descr2);
	}


	// third arg group, not bound to a variable
	{
		args::options_description arg_descr3("Arg group 3");
		arg_descr3.add_options()
			("ee,e", args::value<int>()->default_value(789), "e value")
			(",f", args::value<std::string>()->implicit_value("10"), "f value");
		arg_descr.add(arg_descr3);
	}


	// fourth arg group, with notifier function
	{
		args::options_description arg_descr4("Arg group 4");
		arg_descr4.add_options()
		("notifier", args::value<int>()->composing()->notifier(
			[](auto&& val)
			{
				std::cout << "in notifier function: " << val << std::endl;
			}
		), "value with notifier function")
		(",f", args::value<std::string>()->implicit_value("10"), "f value");
		arg_descr.add(arg_descr4);
	}


	std::cout << arg_descr << std::endl;


	// positional arg group
	args::positional_options_description posarg_descr;
	posarg_descr.add("posarg", -1);


	// argument map
	args::variables_map mapArgs;


	// arg parser
	{
		auto parser = args::command_line_parser(argc, argv);
		parser.style(args::command_line_style::default_style);
		parser.options(arg_descr);
		parser.positional(posarg_descr);
		parser.allow_unregistered();

		// parse args into a map
		auto parsedArgs = parser.run();
		args::store(parsedArgs, mapArgs);

		// unregistered args
		vecUnreg = args::collect_unrecognized(parsedArgs.options, args::exclude_positional);
	}


	// parse args from config file
	try
	{
		auto parsedCfg = args::parse_config_file<char>("tst.cfg", arg_descr);
		args::store(parsedCfg, mapArgs);
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}


	// parse args from environment (with a prefix)
	{
		auto parsedEnv = args::parse_environment(arg_descr, "TST_");
		args::store(parsedEnv, mapArgs);
	}


	args::notify(mapArgs);



	// ------------------------------------------------------------------------
	// print args
	std::cout << "a = " << a << std::endl;
	std::cout << "b = " << b << std::endl;
	std::cout << "c = " << c << std::endl;
	std::cout << "d = " << d << std::endl;
	std::cout << "switch = " << std::boolalpha << bSwitch << std::endl;

	// query unbound args
	if(auto iter = mapArgs.find("e"); iter!=mapArgs.end())
		std::cout << "e = " << iter->second.as<int>() << std::endl;
	if(auto iter = mapArgs.find("f"); iter!=mapArgs.end())
		std::cout << "f = " << iter->second.as<std::string>() << std::endl;

	std::cout << "positional args: ";
	std::copy(vecPosArgs.begin(), vecPosArgs.end(), std::ostream_iterator<std::string>(std::cout, ", "));
	std::cout << std::endl;

	std::cout << "unregistered args: ";
	std::copy(vecUnreg.begin(), vecUnreg.end(), std::ostream_iterator<std::string>(std::cout, ", "));
	std::cout << std::endl;
	// ------------------------------------------------------------------------


	return 0;
}
