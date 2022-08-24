/**
 * simple lexer
 * @author Tobias Weber
 * @date 7-mar-20
 * @license see 'LICENSE.EUPL' file
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <regex>


using t_real = double;


enum class Token : int
{
	TOK_REAL	= 1000,
	TOK_IDENT	= 1001,
	TOK_END		= 1002,

	TOK_INVALID	= 10000,
};


/**
 * find all matching tokens for input string
 */
std::vector<std::pair<int, t_real>> get_matching_tokens(const std::string& str)
{
	std::vector<std::pair<int, t_real>> matches;

	{	// real
		std::regex regex{"[0-9]+(\\.[0-9]*)?"};
		std::smatch smatch;
		if(std::regex_match(str, smatch, regex))
		{
			t_real val{};
			std::istringstream{str} >> val;
			matches.emplace_back(std::make_pair((int)Token::TOK_REAL, val));
		}
	}

	{	// ident
		std::regex regex{"[A-Za-z]+[A-Za-z0-9]*"};
		std::smatch smatch;
		if(std::regex_match(str, smatch, regex))
			matches.emplace_back(std::make_pair((int)Token::TOK_IDENT, 0.));
	}

	{	// tokens represented by themselves
		if(str == "+" || str == "-" || str == "*" || str == "/" ||
			str == "%" || str == "^" || str == "(" || str == ")" || str == ",")
			matches.emplace_back(std::make_pair((int)str[0], 0.));
	}

	/*{	// new line
		if(str == "\n")
			matches.emplace_back(std::make_pair((int)Token::TOK_END, 0.));
	}*/

	//std::cerr << "Input \"" << str << "\" has " << matches.size() << " matches." << std::endl;
	return matches;
}


// ----------------------------------------------------------------------------
// classical lexer interface
static std::istream* g_istr = &std::cin;
static std::string longest_input;

t_real yylval = 0.;
char* yytext = nullptr;

int yylex()
{
	std::string input;
	std::vector<std::pair<int, t_real>> longest_matching;

	// find longest matching token
	while(1)
	{
		char c = g_istr->get();

		if(g_istr->eof())
		{
			//std::cerr << "eof" << std::endl;
			break;
		}
		// if outside any other match...
		if(longest_matching.size() == 0)
		{
			// ...ignore white spaces
			if(c==' ' || c=='\t')
				continue;
			// ...end on new line
			if(c=='\n')
				return (int)Token::TOK_END;
		}

		input += c;
		auto matching = get_matching_tokens(input);
		if(matching.size())
		{
			longest_input = input;
			longest_matching = matching;

			if(g_istr->peek() == std::char_traits<char>::eof() || g_istr->eof())
				break;
		}
		else
		{
			// no more matches
			g_istr->putback(c);
			break;
		}
	}

	if(longest_matching.size() == 0)
	{
		std::cerr << "Invalid input in lexer: \"" << input << "\"." << std::endl;
		return (int)Token::TOK_INVALID;
	}
	if(longest_matching.size() > 1)
	{
		std::cerr << "Warning: Ambiguous match in lexer for token \""
			<< longest_input << "\"." << std::endl;
	}
	if(longest_matching.size())
	{
		yylval = std::get<1>(longest_matching[0]);
		yytext = const_cast<char*>(longest_input.c_str());
		return std::get<0>(longest_matching[0]);
	}

	return (int)Token::TOK_END;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// test
/*
int main(int argc, char **argv)
{
	std::ios::sync_with_stdio(false);

	// get standard input or file input
	std::shared_ptr<std::ifstream> _ifstr;

	if(argc > 1)
	{
		_ifstr = std::make_shared<std::ifstream>(argv[1]);
		if(_ifstr)
		{
			std::cerr << "Cannot open file \"" << argv[1] << "\"." << std::endl;
			return -1;
		}
		g_istr = _ifstr.get();
	}

	while(1)
	{
		int token = yylex();
		std::cout << "Token: " << token;
		if(token < 128 && token >= -128)
			std::cout << " (" << (char)token << ")";
		std::cout << ", lval: " << yylval << std::endl;

		if(token == (int)Token::TOK_INVALID)
			break;
	}

	return 0;
}
*/
// ----------------------------------------------------------------------------
