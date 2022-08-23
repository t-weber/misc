/**
 * LR(1) expression parser via recursive ascent
 *
 * @author Tobias Weber
 * @date 21-aug-2022
 * @license see 'LICENSE.EUPL' file
 *
 * Reference for the algorithm:
 *   https://doi.org/10.1016/0020-0190(88)90061-0
 */

#include "expr_parser_recasc.h"

#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>


/**
 * find all matching tokens for input string
 */
std::vector<Token> ExprParser::get_matching_tokens(const std::string& str)
{
	std::vector<Token> matches;

	if constexpr(std::is_floating_point_v<t_val>)
	{       // real
		std::regex regex{"[0-9]+(\\.[0-9]*)?"};
		std::smatch smatch;
		if(std::regex_match(str, smatch, regex))
		{
			Token tok{};
			tok.id = Token::REAL;
			std::istringstream{str} >> tok.val;
			matches.emplace_back(std::move(tok));
		}
	}
	else if constexpr(std::is_integral_v<t_val>)
	{       // int
		std::regex regex{"[0-9]+"};
		std::smatch smatch;
		if(std::regex_match(str, smatch, regex))
		{
			Token tok{};
			tok.id = Token::REAL;
			std::istringstream{str} >> tok.val;
			matches.emplace_back(std::move(tok));
		}
	}
	else
	{
		std::cerr << "Invalid number type." << std::endl;
	}

	{       // ident
		std::regex regex{"[A-Za-z]+[A-Za-z0-9]*"};
		std::smatch smatch;
		if(std::regex_match(str, smatch, regex))
		{
			Token tok{};
			tok.id = Token::IDENT;
			tok.ident = str;
			matches.emplace_back(std::move(tok));
		}
	}

	{       // tokens represented by themselves
		if(str == "+" || str == "-" || str == "*" || str == "/" || str == "%" ||
			str == "^" || str == "(" || str == ")" || str == ",")
		{
			Token tok{};
			tok.id = (std::size_t)str[0];
			matches.emplace_back(std::move(tok));
		}
	}

	return matches;
}


/**
 * @return [token, yylval, yytext]
 */
Token ExprParser::lex(std::istream* istr)
{
	std::string input, longest_input;
	std::vector<Token> longest_matching;

	// find longest matching token
	while(1)
	{
		char c = istr->get();

		if(istr->eof())
			break;
		// if outside any other match...
		if(longest_matching.size() == 0)
		{
			// ...ignore white spaces
			if(c==' ' || c=='\t')
				continue;
			// ...end on new line
			if(c=='\n')
			{
				Token tok{};
				tok.id = Token::END;
				tok.ident = longest_input;
				return tok;
			}
		}

		input += c;
		auto matching = get_matching_tokens(input);
		if(matching.size())
		{
			longest_input = input;
			longest_matching = std::move(matching);

			if(istr->peek() == std::char_traits<char>::eof() || istr->eof())
				break;
		}
		else
		{
			// no more matches
			istr->putback(c);
			break;
		}
	}

	// at EOF
	if(longest_matching.size() == 0 && input.length() == 0)
	{
		Token tok{};
		tok.id = Token::END;
		tok.ident = longest_input;
		return tok;
	}

	// nothing matches
	if(longest_matching.size() == 0)
	{
		std::cerr << "Invalid input in lexer: \"" << input << "\"." << std::endl;

		Token tok{};
		tok.id = Token::INVALID;
		tok.ident = longest_input;
		return tok;
	}

	// several possible matches
	if(longest_matching.size() > 1)
	{
		std::cerr << "Warning: Ambiguous match in lexer for token \"" << longest_input << "\"." << std::endl;
	}

	// found match
	if(longest_matching.size())
	{
		return longest_matching[0];
	}

	// should not get here
	Token tok{};
	tok.id = Token::INVALID;
	tok.ident = longest_input;
	return tok;
}


void ExprParser::GetNextLookahead()
{
	m_lookahead = lex(m_istr.get());
}


/**
 * get the value of a variable with given id
 */
Symbol ExprParser::GetIdentValue(const std::string& id) const
{
	if(auto iter = m_mapSymbols.find(id); iter != m_mapSymbols.end())
		return Symbol{.is_expr=false, .val=iter->second};

	throw std::runtime_error("Unknown variable \"" + id + "\".");
}


/**
 * call a function with 0 args
 */
Symbol ExprParser::CallFunc(const std::string& id) const
{
	if(auto iter = m_mapFuncs0.find(id); iter != m_mapFuncs0.end())
	{
		t_val retval = (*iter->second)();
		return Symbol{.is_expr=false, .val=retval};
	}

	throw std::runtime_error("Unknown function \"" + id + "\".");
}


/**
 * call a function with 1 arg
 */
Symbol ExprParser::CallFunc(const std::string& id,
	const Symbol& arg) const
{
	if(auto iter = m_mapFuncs1.find(id); iter != m_mapFuncs1.end())
	{
		t_val retval = (*iter->second)(arg.val);
		return Symbol{.is_expr=false, .val=retval};
	}

	throw std::runtime_error("Unknown function \"" + id + "\".");
}


/**
 * call a function with 2 args
 */
Symbol ExprParser::CallFunc(const std::string& id,
	const Symbol& arg1, const Symbol& arg2) const
{
	if(auto iter = m_mapFuncs2.find(id); iter != m_mapFuncs2.end())
	{
		t_val retval = (*iter->second)(arg1.val, arg2.val);
		return Symbol{.is_expr=false, .val=retval};
	}

	throw std::runtime_error("Unknown function \"" + id + "\".");
}


Symbol ExprParser::Parse(const std::string& expr)
{
	m_istr = std::make_shared<std::istringstream>(expr);
	m_lookahead = Token{};
	m_dist_to_jump = 0;
	m_accepted = false;
	while(!m_symbols.empty())
		m_symbols.pop();

	GetNextLookahead();
	start();

	if(m_symbols.size() && m_accepted)
		return m_symbols.top();

	// error
	return Symbol{};
}


/**
 * start -> •expr ｜ end
 */
void ExprParser::start()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_expr();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * start -> expr• ｜ end
 */
void ExprParser::after_expr()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			add_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			sub_after_op();
			break;
		}
		case '*':
		{
			GetNextLookahead();
			mul_after_op();
			break;
		}
		case '/':
		{
			GetNextLookahead();
			div_after_op();
			break;
		}
		case '%':
		{
			GetNextLookahead();
			mod_after_op();
			break;
		}
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}
		case Token::END:
		{
			m_accepted = true;
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr + •expr ｜ ) , end + - * / % ^
 */
void ExprParser::add_after_op()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_add();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr + expr• ｜ ) , end + - * / % ^
 */
void ExprParser::after_add()
{
	switch(m_lookahead.id)
	{
		case '*':
		{
			GetNextLookahead();
			mul_after_op();
			break;
		}
		case '/':
		{
			GetNextLookahead();
			div_after_op();
			break;
		}
		case '%':
		{
			GetNextLookahead();
			mod_after_op();
			break;
		}
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}

		case '+': case '-': case ')': case ',': case Token::END:
		{
			m_dist_to_jump = 3;
			std::vector<Symbol> args(2);
			for(std::size_t arg=0; arg<2; ++arg)
			{
				args[1-arg] = std::move(m_symbols.top());
				m_symbols.pop();
			}

			// semantic rule: expr -> expr + expr.
			m_symbols.emplace(Symbol{.is_expr = true, .val = args[0].val + args[1].val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr - •expr ｜ ) * / % ^ , end + -
 */
void ExprParser::sub_after_op()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_sub();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr - expr• ｜ + end , - * / % ^ )
 */
void ExprParser::after_sub()
{
	switch(m_lookahead.id)
	{
		case '*':
		{
			GetNextLookahead();
			mul_after_op();
			break;
		}
		case '/':
		{
			GetNextLookahead();
			div_after_op();
			break;
		}
		case '%':
		{
			GetNextLookahead();
			mod_after_op();
			break;
		}
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}

		case '+': case '-': case ',': case ')': case Token::END:
		{
			m_dist_to_jump = 3;
			std::vector<Symbol> args(2);
			for(std::size_t arg=0; arg<2; ++arg)
			{
				args[1-arg] = std::move(m_symbols.top());
				m_symbols.pop();
			}

			// semantic rule: expr -> expr - expr.
			m_symbols.emplace(Symbol{.is_expr = true, .val = args[0].val - args[1].val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr * •expr ｜ ) / % ^ , + end - *
 */
void ExprParser::mul_after_op()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_mul();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr * expr• ｜ - * / % , + end ^ )
 */
void ExprParser::after_mul()
{
	switch(m_lookahead.id)
	{
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}

		case '+': case '-': case '*': case '/':
		case '%': case ')': case ',': case Token::END:
		{
			m_dist_to_jump = 3;
			std::vector<Symbol> args(2);
			for(std::size_t arg=0; arg<2; ++arg)
			{
				args[1-arg] = std::move(m_symbols.top());
				m_symbols.pop();
			}

			// semantic rule: expr -> expr * expr.
			m_symbols.emplace(Symbol{.is_expr = true, .val = args[0].val * args[1].val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr / •expr ｜ ) - % ^ , + end * /
 */
void ExprParser::div_after_op()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_div();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr / expr• ｜ * - / % , + end ^ )
 */
void ExprParser::after_div()
{
	switch(m_lookahead.id)
	{
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}

		case '+': case '-': case '*': case '/':
		case '%': case ')': case ',': case Token::END:
		{
			m_dist_to_jump = 3;
			std::vector<Symbol> args(2);
			for(std::size_t arg=0; arg<2; ++arg)
			{
				args[1-arg] = std::move(m_symbols.top());
				m_symbols.pop();
			}

			// semantic rule: expr -> expr / expr.
			m_symbols.emplace(Symbol{.is_expr = true, .val = args[0].val / args[1].val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr % •expr ｜ ) ^ * - , end + / %
 */
void ExprParser::mod_after_op()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT: 
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_mod();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr % expr• ｜ / % ^ * - + end , )
 */
void ExprParser::after_mod()
{
	switch(m_lookahead.id)
	{
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}

		case '+': case '-': case '*': case '/':
		case '%': case ',': case ')': case Token::END:
		{
			m_dist_to_jump = 3;
			std::vector<Symbol> args(2);
			for(std::size_t arg=0; arg<2; ++arg)
			{
				args[1-arg] = std::move(m_symbols.top());
				m_symbols.pop();
			}

			// semantic rule: expr -> expr % expr.
			m_symbols.emplace(Symbol{.is_expr = true, .val = std::fmod(args[0].val, args[1].val)});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr ^ •expr ｜ ) % ^ / * - , + end
 */
void ExprParser::pow_after_op()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_pow();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> expr ^ expr• ｜ % ^ / * - + end , )
 */
void ExprParser::after_pow()
{
	switch(m_lookahead.id)
	{
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}

		case '+': case '-': case '*': case '/':
		case '%': case ',': case ')': case Token::END:
		{
			m_dist_to_jump = 3;
			std::vector<Symbol> args(2);
			for(std::size_t arg=0; arg<2; ++arg)
			{
				args[1-arg] = std::move(m_symbols.top());
				m_symbols.pop();
			}

			// semantic rule: expr -> expr ^ expr.
			m_symbols.emplace(Symbol{.is_expr = true, .val = std::pow(args[0].val, args[1].val)});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ( •expr ) ｜ ) ^ % / * - , + end
 */
void ExprParser::after_bracket()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			bracket_after_expr();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ( expr •) ｜ ^ % / * - + end , )
 */
void ExprParser::bracket_after_expr()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			add_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			sub_after_op();
			break;
		}
		case '*':
		{
			GetNextLookahead();
			mul_after_op();
			break;
		}
		case '/':
		{
			GetNextLookahead();
			div_after_op();
			break;
		}
		case '%':
		{
			GetNextLookahead();
			mod_after_op();
			break;
		}
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}
		case ')':
		{
			GetNextLookahead();
			after_bracket_expr();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ident •( ) ｜ ^ % / * - , end + )
 */
void ExprParser::after_ident()
{
	switch(m_lookahead.id)
	{
		case '(':
		{
			GetNextLookahead();
			funccall_after_ident();
			break;
		}

		case '+': case '-': case '*': case '/':
		case '%': case '^': case ',': case ')':
		case Token::END:
		{
			m_dist_to_jump = 1;
			Symbol arg = std::move(m_symbols.top());
			m_symbols.pop();

			// semantic rule: expr -> ident.
			m_symbols.emplace(Symbol{.is_expr = true, .val = arg.val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ( expr )• ｜ ^ % / * - + end , )
 */
void ExprParser::after_bracket_expr()
{
	switch(m_lookahead.id)
	{
		case '+': case '-': case '*': case '/':
		case '%': case '^': case ',': case ')':
		case Token::END:
		{
			m_dist_to_jump = 3;
			Symbol arg = std::move(m_symbols.top());
			m_symbols.pop();

			// semantic rule: expr -> ( expr ).
			m_symbols.emplace(Symbol{.is_expr = true, .val = arg.val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ident ( •) ｜ ^ % / * - end , + )
 */
void ExprParser::funccall_after_ident()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case ')':
		{
			GetNextLookahead();
			after_funccall_0args();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			funccall_after_arg();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ident ( )• ｜ ^ % / * - end , + )
 */
void ExprParser::after_funccall_0args()
{
	switch(m_lookahead.id)
	{
		case '+': case '-': case '*': case '/':
		case '%': case '^': case ',': case ')':
		case Token::END:
		{
			m_dist_to_jump = 3;
			Symbol arg = std::move(m_symbols.top());
			m_symbols.pop();

			// TODO: semantic rule: expr -> ident ( ).
			//m_symbols.emplace(CallFunc(arg));
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ident ( expr •) ｜ ^ % / * - end , + )
 */
void ExprParser::funccall_after_arg()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			add_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			sub_after_op();
			break;
		}
		case '*':
		{
			GetNextLookahead();
			mul_after_op();
			break;
		}
		case '/':
		{
			GetNextLookahead();
			div_after_op();
			break;
		}
		case '%':
		{
			GetNextLookahead();
			mod_after_op();
			break;
		}
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}
		case ',':
		{
			GetNextLookahead();
			funccall_after_comma();
			break;
		}
		case ')':
		{
			GetNextLookahead();
			after_funccall_1arg();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ident ( expr )• ｜ ^ % / * - end , + )
 */
void ExprParser::after_funccall_1arg()
{
	switch(m_lookahead.id)
	{
		case '+': case '-': case '*': case '/':
		case '%': case '^': case ',': case ')':
		case Token::END:
		{
			m_dist_to_jump = 4;
			std::vector<Symbol> args(2);
			for(std::size_t arg=0; arg<2; ++arg)
			{
				args[1-arg] = std::move(m_symbols.top());
				m_symbols.pop();
			}

			// TODO: semantic rule: expr -> ident ( expr ).
			// m_symbols.emplace(CallFunc(args[0], args[1]));
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ident ( expr , •expr ) ｜ ^ % / * - end , + )
 */
void ExprParser::funccall_after_comma()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			funccall_after_arg2();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ident ( expr , expr •) ｜ ^ % / * - end , + )
 */
void ExprParser::funccall_after_arg2()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			add_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			sub_after_op();
			break;
		}
		case '*':
		{
			GetNextLookahead();
			mul_after_op();
			break;
		}
		case '/':
		{
			GetNextLookahead();
			div_after_op();
			break;
		}
		case '%':
		{
			GetNextLookahead();
			mod_after_op();
			break;
		}
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}
		case ')':
		{
			GetNextLookahead();
			after_funccall_2args();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> real• ｜ ^ % / * - , end + )
 */
void ExprParser::after_real()
{
	switch(m_lookahead.id)
	{
		case '+': case '-': case '*': case '/':
		case '%': case '^': case ',': case ')':
		case Token::END:
		{
			m_dist_to_jump = 1;
			Symbol arg = std::move(m_symbols.top());
			m_symbols.pop();

			// semantic rule: expr -> real.
			m_symbols.emplace(Symbol{.is_expr = true, .val = arg.val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> - •expr ｜ ^ % / * - , end + )
 */
void ExprParser::usub_after_op()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_usub();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> ident ( expr , expr )• ｜ ^ % / * - end , + )
 */
void ExprParser::after_funccall_2args()
{
	switch(m_lookahead.id)
	{
		case '+': case '-': case '*': case '/':
		case '%': case '^': case ',': case ')':
		case Token::END:
		{
			m_dist_to_jump = 6;
			std::vector<Symbol> args(3);
			for(std::size_t arg=0; arg<3; ++arg)
			{
				args[2-arg] = std::move(m_symbols.top());
				m_symbols.pop();
			}

			// TODO: semantic rule: expr -> ident ( expr , expr ).
			// m_symbols.emplace(CallFunc(args[0], args[1], args[2]));
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> - expr• ｜ ^ % / * - end , + )
 */
void ExprParser::after_usub()
{
	switch(m_lookahead.id)
	{
		case '*':
		{
			GetNextLookahead();
			mul_after_op();
			break;
		}
		case '/':
		{
			GetNextLookahead();
			div_after_op();
			break;
		}
		case '%':
		{
			GetNextLookahead();
			mod_after_op();
			break;
		}
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}

		case '+': case '-': case ',': case ')': case Token::END:
		{
			m_dist_to_jump = 2;
			Symbol arg = std::move(m_symbols.top());
			m_symbols.pop();

			// semantic rule: expr -> - expr.
			m_symbols.emplace(Symbol{.is_expr = true, .val = -arg.val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> + •expr ｜ ^ % / * - , end + )
 */
void ExprParser::uadd_after_op()
{
	switch(m_lookahead.id)
	{
		case '+':
		{
			GetNextLookahead();
			uadd_after_op();
			break;
		}
		case '-':
		{
			GetNextLookahead();
			usub_after_op();
			break;
		}
		case '(':
		{
			GetNextLookahead();
			after_bracket();
			break;
		}
		case Token::REAL:
		{
			m_symbols.emplace(Symbol{.is_expr=false, .val=m_lookahead.val});
			GetNextLookahead();
			after_real();
			break;
		}
		case Token::IDENT:
		{
			m_symbols.push(GetIdentValue(m_lookahead.ident));
			GetNextLookahead();
			after_ident();
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	while(!m_dist_to_jump && m_symbols.size() && !m_accepted)
	{
		const Symbol& topsym = m_symbols.top();
		if(topsym.is_expr)
			after_uadd();
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


/**
 * expr -> + expr• ｜ ^ % / * - + end , )
 */
void ExprParser::after_uadd()
{
	switch(m_lookahead.id)
	{
		case '%':
		{
			GetNextLookahead();
			mod_after_op();
			break;
		}
		case '*':
		{
			GetNextLookahead();
			mul_after_op();
			break;
		}
		case '/':
		{
			GetNextLookahead();
			div_after_op();
			break;
		}
		case '^':
		{
			GetNextLookahead();
			pow_after_op();
			break;
		}

		case '+': case '-': case ',': case ')': case Token::END:
		{
			m_dist_to_jump = 2;
			Symbol arg = std::move(m_symbols.top());
			m_symbols.pop();

			// semantic rule: expr -> + expr.
			m_symbols.emplace(Symbol{.is_expr = true, .val = arg.val});
			break;
		}
		default:
		{
			throw std::runtime_error("No transition from " + std::string(__FUNCTION__)
				+ " and look-ahead terminal " + std::to_string(m_lookahead.id) + ".");
			break;
		}
	}

	if(m_dist_to_jump > 0)
		--m_dist_to_jump;
}


int main()
{
	try
	{
		ExprParser parser;
		while(true)
		{
			std::string expr;
			std::cout << "> ";

			std::getline(std::cin, expr);
			if(expr == "")
				continue;

			std::cout << parser.Parse(expr).val << std::endl;
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	return 0;
}
