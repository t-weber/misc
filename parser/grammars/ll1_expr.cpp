/**
 * @author Tobias Weber
 * @date 23-nov-19
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- https://www.cs.uaf.edu/~cs331/notes/FirstFollow.pdf
 *	- https://de.wikipedia.org/wiki/LL(k)-Grammatik
 *
 * flex -o ll1_lexer.cpp ll1_expr.l
 * g++ -o ll1_expr ll1_expr.cpp ll1_lexer.cpp
 */

#include <iostream>
#include <cmath>


#define TOK_REAL 1000
#define TOK_END 1001

using t_real = double;

extern int yylex();
extern t_real yylval;
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


t_real expr();
t_real expr_rest(t_real arg);
t_real factor();
t_real term();
t_real term_rest(t_real arg);


t_real expr()
{
	// expr -> term expr_rest
	if(lookahead == '(' || lookahead == TOK_REAL)
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
	else if(lookahead == ')' || lookahead == TOK_END)
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

	std::cerr << "Invalid lookahead in " << __func__ << ": " << lookahead << "." << std::endl;
	return 0.;
}


t_real term()
{
	// term -> factor term_rest
	if(lookahead == '(' || lookahead == TOK_REAL)
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
	else if(lookahead == '+' || lookahead == '-' || lookahead == ')' || lookahead == TOK_END)
	{
		return arg;
	}

	std::cerr << "Invalid lookahead in " << __func__ << ": " << lookahead << "." << std::endl;
	return 0.;
}



int main()
{
	//std::cout << yylex() << ", " << yylval << std::endl;

	while(1)
	{
		next_lookahead();
		std::cout << expr() << std::endl;
	}

	return 0;
}
