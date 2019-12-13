/**
 * simple LR(1) operator precedence expression test.
 *
 * @author Tobias Weber
 * @date 13-dec-19
 * @license see 'LICENSE.EUPL' file
 *
 * Reference:
 * 	https://www.geeksforgeeks.org/operator-grammar-and-precedence-parser-in-toc/
 *
 * flex -o ll1_lexer.cpp ll1_expr.l && g++ -std=c++17 -o lr1_opprec lr1_opprec.cpp ll1_lexer.cpp
 */

#include <iostream>
#include <stack>
#include <unordered_map>
#include <tuple>
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
// ----------------------------------------------------------------------------



void parse()
{
	auto get_prec_idx = [](int sym1, int sym2) -> int
	{
		return sym1<<16 + sym2;
	};

	// operator precedence table
	std::unordered_map<int, short> prec_tab
	{
		std::make_pair(get_prec_idx('+', '*'), 1),
		std::make_pair(get_prec_idx('*', '+'), -1),
	};


	std::stack<std::tuple<
		int,			// token
		t_real, 		// value
		bool,			// is terminal?
		short>			// precedence
		> stack;


	// shift tokens onto the stack
	auto shift = [&stack](int tok, t_real value, bool is_term, short prec) -> void
	{
		stack.emplace(std::make_tuple(tok, value, is_term, prec));
	};


	// reduce tokens from the stack to a non-terminal
	auto reduce = [&stack]() -> void
	{
		// TODO
	};



	stack.emplace(std::make_tuple(TOK_END, 0, 1, 0));

	while(1)
	{
		if(stack.empty())
		{
			std::cerr << "Stack is empty." << std::endl;
			break;
		}

		int lookahead = yylex();
		const auto& [top_tok, top_val, top_isterm, top_prec] = stack.top();

		auto iterPrec = prec_tab.find(get_prec_idx(lookahead, top_tok));
		if(iterPrec == prec_tab.end())
		{
			std::cerr << "No entry in precedence table for tokens ("
				<< lookahead << ", " << top_tok << ")." << std::endl;
			break;
		}

		// TODO
	}
}



int main()
{
	std::cout.precision(8);
	parse();

	return 0;
}
