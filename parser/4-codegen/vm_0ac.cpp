/**
 * zero-address (stack) machine test
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 *
 * Test:
 *	echo -e "x = (2+3)*(4-5)\n y=x*x+x" | ./parser | ./vm_0ac
 */

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <stack>
#include <unordered_map>
#include <variant>
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
	std::unordered_map<std::string, t_real> syms =
	{{
		{ "pi", M_PI },
	}};

	std::stack<std::variant<t_real, std::string>> stack;


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

		else if(tokens[0] == "PUSHVAL")
		{
			if(tokens.size() < 2)
			{
				std::cerr << "Need variable name to push." << std::endl;
				continue;
			}

			auto iter = syms.find(tokens[1]);
			if(iter != syms.end())
				stack.push(iter->second);
			else
				std::cerr << "Unknown variable: " << tokens[1] << "." << std::endl;
		}

		else if(tokens[0] == "PUSHVAR")
		{
			if(tokens.size() < 2)
			{
				std::cerr << "Need variable name to push." << std::endl;
				continue;
			}

			std::string var = tokens[1];
			stack.emplace(std::move(var));
		}

		else if(tokens[0] == "ASSIGN")
		{
			std::string var = std::get<std::string>(stack.top()); stack.pop();
			t_real val = std::get<t_real>(stack.top()); stack.pop();

			syms[var] = val;
		}

		else if(tokens[0] == "UMIN")
		{
			t_real val = std::get<t_real>(stack.top()); stack.pop();
			stack.push(-val);
		}

		else if(tokens[0] == "ADD")
		{
			t_real val1 = std::get<t_real>(stack.top()); stack.pop();
			t_real val0 = std::get<t_real>(stack.top()); stack.pop();
			stack.push(val0 + val1);
		}

		else if(tokens[0] == "SUB")
		{
			t_real val1 = std::get<t_real>(stack.top()); stack.pop();
			t_real val0 = std::get<t_real>(stack.top()); stack.pop();
			stack.push(val0 - val1);
		}

		else if(tokens[0] == "MUL")
		{
			t_real val1 = std::get<t_real>(stack.top()); stack.pop();
			t_real val0 = std::get<t_real>(stack.top()); stack.pop();
			stack.push(val0 * val1);
		}

		else if(tokens[0] == "DIV")
		{
			t_real val1 = std::get<t_real>(stack.top()); stack.pop();
			t_real val0 = std::get<t_real>(stack.top()); stack.pop();
			stack.push(val0 / val1);
		}

		else if(tokens[0] == "MOD")
		{
			t_real val1 = std::get<t_real>(stack.top()); stack.pop();
			t_real val0 = std::get<t_real>(stack.top()); stack.pop();
			stack.push(std::fmod(val0, val1));
		}

		else if(tokens[0] == "POW")
		{
			t_real val1 = std::get<t_real>(stack.top()); stack.pop();
			t_real val0 = std::get<t_real>(stack.top()); stack.pop();
			stack.push(std::pow(val0, val1));
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
				t_real val = std::get<t_real>(stack.top()); stack.pop();
				stack.push(std::sin(val));
			}
			else
			{
				std::cerr << "Unknown function: " << tokens[1] << "." << std::endl;
			}
		}
	}


	std::cout << "End of program.\n";

	std::cout << "\nSymbols:\n";
	for(const auto& sym : syms)
		std::cout << "\t" << sym.first << " = " << sym.second << std::endl;

	if(!stack.empty())
	{
		std::cout << "\nStack contents:\n";
		while(stack.size())
		{
			t_real val = std::get<t_real>(stack.top());
			std::cout << "\t" << val << std::endl;
			stack.pop();
		}
	}
}


int main()
{
	run(std::cin);

	return 0;
}
