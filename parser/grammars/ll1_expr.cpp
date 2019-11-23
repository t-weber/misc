/**
 * simple LL(1) expression test,
 * see ll1.cpp for grammar and calculation of first/follow sets.
 *
 * @author Tobias Weber
 * @date 23-nov-19
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- https://www.cs.uaf.edu/~cs331/notes/FirstFollow.pdf
 *	- https://de.wikipedia.org/wiki/LL(k)-Grammatik
 *
 * flex -o ll1_lexer.cpp ll1_expr.l && g++ -std=c++17 -o ll1_expr ll1_expr.cpp ll1_lexer.cpp
 */

#include <iostream>
#include <unordered_map>
#include <cmath>

using t_real = double;


// ----------------------------------------------------------------------------
// Lexer interface
// ----------------------------------------------------------------------------

#define TOK_REAL	1000
#define TOK_IDENT	1001
#define TOK_END		1002

extern int yylex();
extern t_real yylval;
extern char* yytext;
static int lookahead = -1;


static int next_lookahead()
{
	return lookahead = yylex();
}


static bool match(int expected)
{
	if(lookahead != expected)
	{
		std::cerr << "Could not match symbol! Expected: "
			<< expected << ", got: " << lookahead << "." << std::endl;

		return false;
	}

	return true;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// Tables
// ----------------------------------------------------------------------------

// symbol table
static std::unordered_map<std::string, t_real> g_mapSymbols =
{
	{ "pi", M_PI },
};


// zero-args function table
static std::unordered_map<std::string, t_real(*)()> g_mapFuncs0 =
{
};


// one-arg function table
static std::unordered_map<std::string, t_real(*)(t_real)> g_mapFuncs1 =
{
	{ "sin", std::sin },
	{ "cos", std::cos },
	{ "tan", std::tan },

	{ "sqrt", std::sqrt },
	{ "exp", std::exp },
};


// two-args function table
static std::unordered_map<std::string, t_real(*)(t_real, t_real)> g_mapFuncs2 =
{
	{ "pow", std::pow },
};
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// Productions
// ----------------------------------------------------------------------------

t_real expr();
t_real expr_rest(t_real arg);
t_real factor();
t_real term();
t_real term_rest(t_real arg);


t_real expr()
{
	// expr -> term expr_rest
	if(lookahead == '(' || lookahead == TOK_REAL || lookahead == TOK_IDENT)
	{
		t_real term_val = term();
		t_real expr_rest_val = expr_rest(term_val);

		return expr_rest_val;
	}
	else if(lookahead == '+')	// unary +
	{
		next_lookahead();
		t_real term_val = term();
		t_real expr_rest_val = expr_rest(term_val);

		return expr_rest_val;
	}
	else if(lookahead == '-')	// unary -
	{
		next_lookahead();
		t_real term_val = -term();
		t_real expr_rest_val = expr_rest(term_val);

		return expr_rest_val;
	}

	std::cerr << "Invalid lookahead in " << __func__ << ": " << lookahead << "." << std::endl;
	return 0.;
}


t_real expr_rest(t_real arg)
{
	// expr_rest -> '+' term expr_rest
	if(lookahead == '+')
	{
		next_lookahead();
		t_real term_val = arg + term();
		t_real expr_rest_val = expr_rest(term_val);

		return expr_rest_val;
	}

	// expr_rest -> '-' term expr_rest
	else if(lookahead == '-')
	{
		next_lookahead();
		t_real term_val = arg - term();
		t_real expr_rest_val = expr_rest(term_val);

		return expr_rest_val;
	}
	// expr_rest -> epsilon
	else if(lookahead == ')' || lookahead == TOK_END || lookahead == ',')
	{
		return arg;
	}

	std::cerr << "Invalid lookahead in " << __func__ << ": " << lookahead << "." << std::endl;
	return 0.;
}


t_real factor()
{
	// factor -> '(' expr ')'
	if(lookahead == '(')
	{
		next_lookahead();
		t_real expr_val = expr();
		match(')');
		next_lookahead();

		return expr_val;
	}

	// factor -> TOK_REAL
	else if(lookahead == TOK_REAL)
	{
		t_real val = yylval;
		next_lookahead();

		return val;
	}

	// factor -> TOK_IDENT
	else if(lookahead == TOK_IDENT)
	{
		std::string ident = yytext;
		next_lookahead();

		// function call
		// using next lookahead, grammar still ll(1)?
		if(lookahead == '(')
		{
			next_lookahead();

			// 0-argument function
			// factor -> TOK_IDENT '(' ')'
			if(lookahead == ')')
			{
				next_lookahead();

				auto iter = g_mapFuncs0.find(ident);
				if(iter == g_mapFuncs0.end())
				{
					std::cerr << "Unknown function \"" << ident << "\"." << std::endl;
					return 0.;
				}

				return (*iter->second)();
			}

			// function with arguments
			else
			{
				// first argument
				t_real expr_val1 = expr();

				// one-argument-function
				// factor -> TOK_IDENT '(' expr ')'
				if(lookahead == ')')
				{
					next_lookahead();

					auto iter = g_mapFuncs1.find(ident);
					if(iter == g_mapFuncs1.end())
					{
						std::cerr << "Unknown function \"" << ident << "\"." << std::endl;
						return 0.;
					}

					return (*iter->second)(expr_val1);
				}

				// two-argument-function
				// factor -> TOK_IDENT '(' expr ',' expr ')'
				else if(lookahead == ',')
				{
					next_lookahead();
					t_real expr_val2 = expr();
					match(')');
					next_lookahead();

					auto iter = g_mapFuncs2.find(ident);
					if(iter == g_mapFuncs2.end())
					{
						std::cerr << "Unknown function \"" << ident << "\"." << std::endl;
						return 0.;
					}

					return (*iter->second)(expr_val1, expr_val2);
				}
				else
				{
					std::cerr << "Invalid function call to \"" << ident << "\"." << std::endl;
				}
			}
		}

		// variable lookup
		else
		{
			auto iter = g_mapSymbols.find(ident);
			if(iter == g_mapSymbols.end())
			{
				std::cerr << "Unknown identifier \"" << ident << "\"." << std::endl;
				return 0.;
			}

			return iter->second;
		}
	}

	std::cerr << "Invalid lookahead in " << __func__ << ": " << lookahead << "." << std::endl;
	return 0.;
}


t_real term()
{
	// term -> factor term_rest
	if(lookahead == '(' || lookahead == TOK_REAL || lookahead == TOK_IDENT)
	{
		t_real factor_val = factor();
		t_real term_rest_val = term_rest(factor_val);

		return term_rest_val;
	}

	std::cerr << "Invalid lookahead in " << __func__ << ": " << lookahead << "." << std::endl;
	return 0.;
}


t_real term_rest(t_real arg)
{
	// term_rest -> '*' factor term_rest
	if(lookahead == '*')
	{
		next_lookahead();
		t_real factor_val = arg * factor();
		t_real term_rest_val = term_rest(factor_val);

		return term_rest_val;
	}

	// term_rest -> '/' factor term_rest
	else if(lookahead == '/')
	{
		next_lookahead();
		t_real factor_val = arg / factor();
		t_real term_rest_val = term_rest(factor_val);

		return term_rest_val;
	}

	// term_rest -> '%' factor term_rest
	else if(lookahead == '%')
	{
		next_lookahead();
		t_real factor_val = std::fmod(arg, factor());
		t_real term_rest_val = term_rest(factor_val);

		return term_rest_val;
	}

	// term_rest -> epsilon
	else if(lookahead == '+' || lookahead == '-' || lookahead == ')' || lookahead == TOK_END || lookahead == ',')
	{
		return arg;
	}

	std::cerr << "Invalid lookahead in " << __func__ << ": " << lookahead << "." << std::endl;
	return 0.;
}
// ----------------------------------------------------------------------------



int main()
{
	//std::cout << yylex() << ", " << yylval << " (" << yytext << ")" << std::endl;
	std::cout.precision(8);

	while(1)
	{
		next_lookahead();
		std::cout << expr() << std::endl;
	}

	return 0;
}
