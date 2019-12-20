/**
 * zero-address (stack) machine test
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 *
 * Test:
 * 	echo -e "(2+3)*(4-5)" | ./parser | ./vm_0ac
 */

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <stack>
#include <cmath>

using t_real = double;


template<class t_str=std::string, template<class...> class t_cont=std::vector>
t_cont<t_str> tokenise(const t_str& _str, const t_str& strSeparators=" \t")
{
	t_cont<t_str> vec;

	// separator predicate
	auto is_sep = [&strSeparators](auto c) -> bool
	{
		for(auto csep : strSeparators)
			if(c == csep)
				return true;
			return false;
	};

	// trim whitespaces
	t_str str = boost::trim_copy_if(_str, is_sep);
	boost::split(vec, str, is_sep, boost::token_compress_on);

	return vec;
}


void run(std::istream& istr)
{
	std::stack<t_real> stack;

	const std::string white{" \t"};
	std::string line;

	while(istr)
	{
		std::getline(istr, line);
		boost::trim(line);
		if(line == "")
			continue;

		std::vector<std::string> tokens;
		tokens = tokenise<std::string, std::vector>(line, white);

		if(tokens.size() == 0)
			continue;

		else if(tokens[0] == "PUSH")
		{
			if(tokens.size() < 2)
			{
				std::cerr << "Need argument to push." << std::endl;
				continue;
			}

			t_real val = std::stod(tokens[1]);
			stack.push(val);
		}

		else if(tokens[0] == "UMIN")
		{
			t_real val = stack.top(); stack.pop();
			stack.push(-val);
		}

		else if(tokens[0] == "ADD")
		{
			t_real val1 = stack.top(); stack.pop();
			t_real val0 = stack.top(); stack.pop();
			stack.push(val0 + val1);
		}

		else if(tokens[0] == "SUB")
		{
			t_real val1 = stack.top(); stack.pop();
			t_real val0 = stack.top(); stack.pop();
			stack.push(val0 - val1);
		}

		else if(tokens[0] == "MUL")
		{
			t_real val1 = stack.top(); stack.pop();
			t_real val0 = stack.top(); stack.pop();
			stack.push(val0 * val1);
		}

		else if(tokens[0] == "DIV")
		{
			t_real val1 = stack.top(); stack.pop();
			t_real val0 = stack.top(); stack.pop();
			stack.push(val0 / val1);
		}

		else if(tokens[0] == "MOD")
		{
			t_real val1 = stack.top(); stack.pop();
			t_real val0 = stack.top(); stack.pop();
			stack.push(std::fmod(val0, val1));
		}

		else if(tokens[0] == "POW")
		{
			t_real val1 = stack.top(); stack.pop();
			t_real val0 = stack.top(); stack.pop();
			stack.push(std::pow(val0, val1));
		}

		else if(tokens[0] == "PUSHVAR")
		{
			if(tokens.size() < 2)
			{
				std::cerr << "Need variable name to push." << std::endl;
				continue;
			}

			if(tokens[1] == "pi")
				stack.push(M_PI);
			else
				std::cerr << "Unknown variable: " << tokens[1] << "." << std::endl;
		}

		else if(tokens[0] == "CALL")
		{
			if(tokens.size() < 3)
			{
				std::cerr << "Need function name and argument count for call." << std::endl;
				continue;
			}

			int argCnt = std::stoi(tokens[2]);
			if(argCnt==1 && tokens[1]=="sin")
			{
				t_real val = stack.top(); stack.pop();
				stack.push(std::sin(val));
			}
			else
			{
				std::cerr << "Unknown function: " << tokens[1] << "." << std::endl;
			}
		}
	}


	std::cout << "End of program. Stack contents:\n";
	while(stack.size())
	{
		t_real val = stack.top();
		std::cout << val << std::endl;
		stack.pop();
	}
}


int main()
{
	run(std::cin);

	return 0;
}
