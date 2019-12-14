/**
 * simple LR(1) operator precedence expression test.
 *
 * @author Tobias Weber
 * @date 14-dec-19
 * @license see 'LICENSE.EUPL' file
 *
 * Reference:
 * 	https://www.geeksforgeeks.org/operator-grammar-and-precedence-parser-in-toc/
 *
 * flex -o ll1_lexer.cpp ll1_expr.l && g++ -std=c++17 -o lr1_opprec lr1_opprec.cpp ll1_lexer.cpp
 */

#include <iostream>
#include <stack>
#include <vector>
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

#define NONTERM_EXPR	2000


std::tuple<bool, t_real> parse(bool bDebug=0)
{
	auto get_prec_idx = [](int sym1, int sym2) -> int
	{
		// use operator equivalent in precedence
		if(sym1 == '-') sym1 = '+';
		else if(sym1 == '/') sym1 = '*';
		else if(sym1 == '%') sym1 = '*';

		if(sym2 == '-') sym2 = '+';
		else if(sym2 == '/') sym2 = '*';
		else if(sym2 == '%') sym2 = '*';

		return (sym1<<16) + sym2;
	};


	auto symmetrise = [&get_prec_idx](auto& map)
	{
		std::remove_reference_t<decltype(map)> mapNewEntries;

		for(const auto& pair : map)
		{
			int sym1 = (0xffff0000 & pair.first) >> 16;
			int sym2 = (0x0000ffff & pair.first);

			// exchange symbols and sign of precedence
			if(sym1 != sym2)
				mapNewEntries.insert(std::make_pair(get_prec_idx(sym2, sym1), -pair.second));
		}

		map.merge(mapNewEntries);
	};


	// operator precedence table
	std::unordered_map<int, short> prec_tab
	{{
		std::make_pair(get_prec_idx('+', '+'), -1),
		std::make_pair(get_prec_idx('+', '*'), +1),
		std::make_pair(get_prec_idx('+', TOK_REAL), +1),
		std::make_pair(get_prec_idx('+', TOK_IDENT), +1),
		std::make_pair(get_prec_idx('+', TOK_END), -1),

		std::make_pair(get_prec_idx('*', '*'), -1),
		std::make_pair(get_prec_idx('*', TOK_REAL), +1),
		std::make_pair(get_prec_idx('*', TOK_IDENT), +1),
		std::make_pair(get_prec_idx('*', TOK_END), -1),

		std::make_pair(get_prec_idx('/', '/'), -1),
		std::make_pair(get_prec_idx('/', TOK_REAL), +1),
		std::make_pair(get_prec_idx('/', TOK_IDENT), +1),
		std::make_pair(get_prec_idx('/', TOK_END), -1),

		std::make_pair(get_prec_idx(TOK_REAL, TOK_END), -1),
		std::make_pair(get_prec_idx(TOK_IDENT, TOK_END), -1),
	}};

	symmetrise(prec_tab);


	using t_stackelem = std::tuple<
		int,			// 0: token
		t_real, 		// 1: value
		bool,			// 2: is terminal?
		short>;			// 3: precedence
	std::stack<t_stackelem> stack;


	// shift tokens onto the stack
	auto shift = [&stack](int tok, t_real value, bool is_term, short prec) -> void
	{
		stack.emplace(std::make_tuple(tok, value, is_term, prec));
	};


	// reduce tokens from the stack to a non-terminal
	auto reduce = [&stack, bDebug]() -> std::tuple<t_real, std::size_t>
	{
		std::vector<t_stackelem> vecreduce;

		while(1)
		{
			if(stack.empty())
			{
				std::cerr << "Stack is empty." << std::endl;
				break;
			}

			t_stackelem stackelem = stack.top();
			vecreduce.push_back(stackelem);
			stack.pop();

			bool is_term = std::get<2>(stackelem);
			short prec = std::get<3>(stackelem);

			if(bDebug)
			{
				std::cerr << "in reduce(): stack element " << std::get<0>(stackelem)
					<< ", precedence " << prec << std::endl;
			}

			// finished?
			if(prec > 0 && is_term)
			{
				// non-terminal on top now?
				if(stack.size()>=1 && std::get<2>(stack.top())==0)
				{
					t_stackelem stackelem_nonterm = stack.top();
					vecreduce.push_back(stackelem_nonterm);
					stack.pop();
				}

				break;
			}
		}

		t_real result = 0;

		if(vecreduce.size()==3 &&
			std::get<0>(vecreduce[0])==NONTERM_EXPR &&
			std::get<0>(vecreduce[2])==NONTERM_EXPR)
		{
			if(std::get<0>(vecreduce[1])=='+')
				result = std::get<1>(vecreduce[2]) + std::get<1>(vecreduce[0]);
			else if(std::get<0>(vecreduce[1])=='-')
				result = std::get<1>(vecreduce[2]) - std::get<1>(vecreduce[0]);
			else if(std::get<0>(vecreduce[1])=='*')
				result = std::get<1>(vecreduce[2]) * std::get<1>(vecreduce[0]);
			else if(std::get<0>(vecreduce[1])=='/')
				result = std::get<1>(vecreduce[2]) / std::get<1>(vecreduce[0]);
			else if(std::get<0>(vecreduce[1])=='%')
				result = std::fmod(std::get<1>(vecreduce[2]), std::get<1>(vecreduce[0]));
			else if(std::get<0>(vecreduce[1])=='^')
				result = std::pow(std::get<1>(vecreduce[2]), std::get<1>(vecreduce[0]));
		}
		else if(vecreduce.size()==1 && std::get<0>(vecreduce[0])==TOK_REAL)
		{
			result = std::get<1>(vecreduce[0]);
		}
		else
		{
			std::cerr << "No production rule found for reduction." << std::endl;
		}

		// directly push result for the moment (TODO: build AST here)
		stack.emplace(std::make_tuple(NONTERM_EXPR, result, 0, 0));

		return std::make_tuple(result, vecreduce.size());
	};



	stack.emplace(std::make_tuple(TOK_END, 0, 1, -1));
	int lookahead = yylex();

	t_real lastResult = 0;
	bool bOk = 0;
	while(1)
	{
		if(stack.empty())
		{
			std::cerr << "Stack is empty." << std::endl;
			break;
		}

		auto [top_tok, top_val, top_isterm, top_prec] = stack.top();
		if(!top_isterm)
		{
			auto stack2 = stack;

			// find next terminal on stack for precedence
			while(1)
			{
				stack2.pop();
				const auto& [top2_tok, top2_val, top2_isterm, top2_prec] = stack2.top();
				if(top2_isterm)
				{
					top_tok = top2_tok;
					top_prec = top2_prec;
					break;
				}
			}
		}

		auto iterPrec = prec_tab.find(get_prec_idx(top_tok, lookahead));

		// accept
		if(lookahead == TOK_END && top_tok == TOK_END)
		{
			bOk = 1;
			if(bDebug)
				std::cerr << "accept" << std::endl;
			break;
		}

		// error
		if(iterPrec == prec_tab.end())
		{
			std::cerr << "No entry in precedence table for tokens ("
				<< lookahead << ", " << top_tok << ")." << std::endl;
			break;
		}

		if(bDebug)
		{
			std::cerr << "lookahead: " << lookahead << ", stack: " << top_tok
				<< ", precedence: " << iterPrec->second
				<< ", stack size: " << stack.size() << std::endl;
		}

		// shift
		if(iterPrec->second >= 0)
		{
			if(bDebug)
				std::cerr << "shifting token " << lookahead << " (" << char(lookahead) << ")" << std::endl;

			shift(lookahead, yylval, 1, iterPrec->second);
			lookahead = yylex();
		}
		// reduce
		else
		{
			auto [result, numreduced] = reduce();
			lastResult = result;

			if(bDebug)
				std::cerr << "reducing " << numreduced << " elements with result: " << result << std::endl;
		}
	}

	return std::make_tuple(bOk, lastResult);
}



int main()
{
	std::cout.precision(8);
	auto [accepted, result] = parse();
	if(accepted)
		std::cout << result << std::endl;
	else
		std::cerr << "Parsing ended with error." << std::endl;

	return 0;
}
