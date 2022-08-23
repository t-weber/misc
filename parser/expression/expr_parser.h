/**
 * simple LL(1) expression parser via recursive descent
 *
 * @author Tobias Weber
 * @date 14-mar-20
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- https://www.cs.uaf.edu/~cs331/notes/FirstFollow.pdf
 *	- https://de.wikipedia.org/wiki/LL(k)-Grammatik
 */

#ifndef __LL1_EXPR_PARSER_H__
#define __LL1_EXPR_PARSER_H__


#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cmath>


template<typename t_real=double>
class ExprParser
{
public:
	t_real parse(const std::string& str)
	{
		m_istr = std::make_shared<std::istringstream>(str);
		next_lookahead();
		return plus_term();
	}


	const std::unordered_map<std::string, t_real>& get_symbols() const
	{
		return m_mapSymbols;
	}


protected:
	// ------------------------------------------------------------------------
	// Lexer
	// ------------------------------------------------------------------------
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

		if constexpr(std::is_floating_point_v<t_real>)
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
		else if constexpr(std::is_integral_v<t_real>)
		{	// int
			std::regex regex{"[0-9]+"};
			std::smatch smatch;
			if(std::regex_match(str, smatch, regex))
			{
				t_real val{};
				std::istringstream{str} >> val;
				matches.emplace_back(std::make_pair((int)Token::TOK_REAL, val));
			}
		}
		else
		{
			std::cerr << "Invalid number type." << std::endl;
		}

		{	// ident
			std::regex regex{"[A-Za-z]+[A-Za-z0-9]*"};
			std::smatch smatch;
			if(std::regex_match(str, smatch, regex))
				matches.emplace_back(std::make_pair((int)Token::TOK_IDENT, 0.));
		}

		{	// tokens represented by themselves
			if(str == "+" || str == "-" || str == "*" || str == "/" || str == "%" ||
				str == "^" || str == "(" || str == ")" || str == "," || str == "=")
				matches.emplace_back(std::make_pair((int)str[0], 0.));
		}

		return matches;
	}


	/**
	 * @return [token, yylval, yytext]
	 */
	std::tuple<int, t_real, std::string> lex()
	{
		std::string input, longest_input;
		std::vector<std::pair<int, t_real>> longest_matching;

		// find longest matching token
		while(1)
		{
			char c = m_istr->get();

			if(m_istr->eof())
				break;
			// if outside any other match...
			if(longest_matching.size() == 0)
			{
				// ...ignore white spaces
				if(c==' ' || c=='\t')
					continue;
				// ...end on new line
				if(c=='\n')
					return std::make_tuple((int)Token::TOK_END, t_real{0}, longest_input);
			}

			input += c;
			auto matching = get_matching_tokens(input);
			if(matching.size())
			{
				longest_input = input;
				longest_matching = matching;

				if(m_istr->peek() == std::char_traits<char>::eof() || m_istr->eof())
					break;
			}
			else
			{
				// no more matches
				m_istr->putback(c);
				break;
			}
		}

		// at EOF
		if(longest_matching.size() == 0 && input.length() == 0)
		{
			return std::make_tuple((int)Token::TOK_END, t_real{0}, longest_input);
		}

		// nothing matches
		if(longest_matching.size() == 0)
		{
			std::cerr << "Invalid input in lexer: \"" << input << "\"." << std::endl;
			return std::make_tuple((int)Token::TOK_INVALID, t_real{0}, longest_input);
		}

		// several possible matches
		if(longest_matching.size() > 1)
		{
			std::cerr << "Warning: Ambiguous match in lexer for token \"" << longest_input << "\"." << std::endl;
		}

		// found match
		if(longest_matching.size())
		{
			return std::make_tuple((int)std::get<0>(longest_matching[0]), std::get<1>(longest_matching[0]), longest_input);
		}

		// should not get here
		return std::make_tuple((int)Token::TOK_INVALID, t_real{0}, longest_input);
	}
	// ------------------------------------------------------------------------



	// ----------------------------------------------------------------------------
	// Lexer interface
	// ----------------------------------------------------------------------------
	void next_lookahead()
	{
		std::tie(m_lookahead, m_lookahead_val, m_lookahead_text) = lex();
		//std::cout << "Next lookahead: " << m_lookahead << " (" << m_lookahead_text << ")" << std::endl;
	}


	bool match(int expected)
	{
		if(m_lookahead != expected)
		{
			std::cerr << "Could not match symbol! Expected: "
				<< expected << ", got: " << m_lookahead << "." << std::endl;

			return false;
		}

		return true;
	}
	// ----------------------------------------------------------------------------



	// ----------------------------------------------------------------------------
	// Productions
	// ----------------------------------------------------------------------------
	/**
	 * +,- terms
	 * (lowest precedence, 1)
	 */
	t_real plus_term()
	{
		// plus_term -> mul_term plus_term_rest
		if(m_lookahead == '(' || m_lookahead == (int)Token::TOK_REAL || m_lookahead == (int)Token::TOK_IDENT)
		{
			t_real term_val = mul_term();
			t_real expr_rest_val = plus_term_rest(term_val);

			return expr_rest_val;
		}
		else if(m_lookahead == '+')	// unary +
		{
			next_lookahead();
			t_real term_val = mul_term();
			t_real expr_rest_val = plus_term_rest(term_val);

			return expr_rest_val;
		}
		else if(m_lookahead == '-')	// unary -
		{
			next_lookahead();
			t_real term_val = -mul_term();
			t_real expr_rest_val = plus_term_rest(term_val);

			return expr_rest_val;
		}

		if(m_lookahead == 0 || m_lookahead == EOF)
			exit(0);

		std::cerr << "Invalid lookahead in " << __func__ << ": " << m_lookahead << "." << std::endl;
		return 0.;
	}


	t_real plus_term_rest(t_real arg)
	{
		// plus_term_rest -> '+' mul_term plus_term_rest
		if(m_lookahead == '+')
		{
			next_lookahead();
			t_real term_val = arg + mul_term();
			t_real expr_rest_val = plus_term_rest(term_val);

			return expr_rest_val;
		}

		// plus_term_rest -> '-' mul_term plus_term_rest
		else if(m_lookahead == '-')
		{
			next_lookahead();
			t_real term_val = arg - mul_term();
			t_real expr_rest_val = plus_term_rest(term_val);

			return expr_rest_val;
		}
		// plus_term_rest -> epsilon
		else if(m_lookahead == ')' || m_lookahead == (int)Token::TOK_END || m_lookahead == ',')
		{
			return arg;
		}

		std::cerr << "Invalid lookahead in " << __func__ << ": " << m_lookahead << "." << std::endl;
		return 0.;
	}


	/**
	 * *,/,% terms
	 * (precedence 2)
	 */
	t_real mul_term()
	{
		// mul_term -> pow_term mul_term_rest
		if(m_lookahead == '(' || m_lookahead == (int)Token::TOK_REAL || m_lookahead == (int)Token::TOK_IDENT)
		{
			t_real factor_val = pow_term();
			t_real term_rest_val = mul_term_rest(factor_val);

			return term_rest_val;
		}

		std::cerr << "Invalid lookahead in " << __func__ << ": " << m_lookahead << "." << std::endl;
		return 0.;
	}


	t_real mul_term_rest(t_real arg)
	{
		// mul_term_rest -> '*' pow_term mul_term_rest
		if(m_lookahead == '*')
		{
			next_lookahead();
			t_real factor_val = arg * pow_term();
			t_real term_rest_val = mul_term_rest(factor_val);

			return term_rest_val;
		}

		// mul_term_rest -> '/' pow_term mul_term_rest
		else if(m_lookahead == '/')
		{
			next_lookahead();
			t_real factor_val = arg / pow_term();
			t_real term_rest_val = mul_term_rest(factor_val);

			return term_rest_val;
		}

		// mul_term_rest -> '%' pow_term mul_term_rest
		else if(m_lookahead == '%')
		{
			next_lookahead();
			t_real factor_val = std::fmod(arg, pow_term());
			t_real term_rest_val = mul_term_rest(factor_val);

			return term_rest_val;
		}

		// mul_term_rest -> epsilon
		else if(m_lookahead == '+' || m_lookahead == '-' || m_lookahead == ')'
			|| m_lookahead == (int)Token::TOK_END || m_lookahead == ',')
		{
			return arg;
		}

		std::cerr << "Invalid lookahead in " << __func__ << ": " << m_lookahead << "." << std::endl;
		return 0.;
	}


	/**
	 * ^ terms
	 * (precedence 3)
	 */
	t_real pow_term()
	{
		// pow_term -> factor pow_term_rest
		if(m_lookahead == '(' || m_lookahead == (int)Token::TOK_REAL || m_lookahead == (int)Token::TOK_IDENT)
		{
			t_real factor_val = factor();
			t_real term_rest_val = pow_term_rest(factor_val);

			return term_rest_val;
		}

		std::cerr << "Invalid lookahead in " << __func__ << ": " << m_lookahead << "." << std::endl;
		return 0.;
	}


	t_real pow_term_rest(t_real arg)
	{
		// pow_term_rest -> '^' factor pow_term_rest
		if(m_lookahead == '^')
		{
			next_lookahead();
			t_real factor_val = std::pow(arg, factor());
			t_real term_rest_val = pow_term_rest(factor_val);

			return term_rest_val;
		}

		// pow_term_rest -> epsilon
		else if(m_lookahead == '+' || m_lookahead == '-' || m_lookahead == ')'
			|| m_lookahead == (int)Token::TOK_END || m_lookahead == ','
			|| m_lookahead == '*' || m_lookahead == '/' || m_lookahead == '%')
		{
			return arg;
		}

		std::cerr << "Invalid lookahead in " << __func__ << ": " << m_lookahead << "." << std::endl;
		return 0.;
	}


	/**
	 * () terms, real factor or identifier
	 * (highest precedence, 4)
	 */
	t_real factor()
	{
		// factor -> '(' plus_term ')'
		if(m_lookahead == '(')
		{
			next_lookahead();
			t_real expr_val = plus_term();
			match(')');
			next_lookahead();

			return expr_val;
		}

		// factor -> TOK_REAL
		else if(m_lookahead == (int)Token::TOK_REAL)
		{
			t_real val = m_lookahead_val;
			next_lookahead();

			return val;
		}

		// factor -> TOK_IDENT
		else if(m_lookahead == (int)Token::TOK_IDENT)
		{
			const std::string ident = m_lookahead_text;
			next_lookahead();

			// function call
			// using next m_lookahead, grammar still ll(1)?
			if(m_lookahead == '(')
			{
				next_lookahead();

				// 0-argument function
				// factor -> TOK_IDENT '(' ')'
				if(m_lookahead == ')')
				{
					next_lookahead();

					auto iter = m_mapFuncs0.find(ident);
					if(iter == m_mapFuncs0.end())
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
					t_real expr_val1 = plus_term();

					// one-argument-function
					// factor -> TOK_IDENT '(' plus_term ')'
					if(m_lookahead == ')')
					{
						next_lookahead();

						auto iter = m_mapFuncs1.find(ident);
						if(iter == m_mapFuncs1.end())
						{
							std::cerr << "Unknown function \"" << ident << "\"." << std::endl;
							return 0.;
						}

						return (*iter->second)(expr_val1);
					}

					// two-argument-function
					// factor -> TOK_IDENT '(' plus_term ',' plus_term ')'
					else if(m_lookahead == ',')
					{
						next_lookahead();
						t_real expr_val2 = plus_term();
						match(')');
						next_lookahead();

						auto iter = m_mapFuncs2.find(ident);
						if(iter == m_mapFuncs2.end())
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

			// assignment
			else if(m_lookahead == '=')
			{
				next_lookahead();
				t_real assign_val = plus_term();
				m_mapSymbols[ident] = assign_val;
				return assign_val;
			}

			// variable lookup
			else
			{
				auto iter = m_mapSymbols.find(ident);
				if(iter == m_mapSymbols.end())
				{
					std::cerr << "Unknown identifier \"" << ident << "\"." << std::endl;
					return 0.;
				}

				return iter->second;
			}
		}

		std::cerr << "Invalid lookahead in " << __func__ << ": " << m_lookahead << "." << std::endl;
		return 0.;
	}
	// ----------------------------------------------------------------------------


private:
	std::shared_ptr<std::istream> m_istr{};

	int m_lookahead = (int)Token::TOK_INVALID;
	t_real m_lookahead_val{};
	std::string m_lookahead_text{};


	// ----------------------------------------------------------------------------
	// Tables
	// ----------------------------------------------------------------------------
	// symbol table
	std::unordered_map<std::string, t_real> m_mapSymbols =
	{
		{ "pi", M_PI },
	};


	// zero-args function table
	std::unordered_map<std::string, t_real(*)()> m_mapFuncs0 =
	{
	};


	// one-arg function table
	std::unordered_map<std::string, t_real(*)(t_real)> m_mapFuncs1 =
	{
		{ "sin", [](t_real x) -> t_real { return (t_real)std::sin(x); } },
		{ "cos", [](t_real x) -> t_real { return (t_real)std::cos(x); } },
		{ "tan", [](t_real x) -> t_real { return (t_real)std::tan(x); } },

		{ "sqrt", [](t_real x) -> t_real { return (t_real)std::sqrt(x); } },
		{ "exp", [](t_real x) -> t_real { return (t_real)std::exp(x); } },
	};


	// two-args function table
	std::unordered_map<std::string, t_real(*)(t_real, t_real)> m_mapFuncs2 =
	{
		{ "pow", [](t_real x, t_real y) -> t_real { return (t_real)std::pow(x, y); } },
	};
	// ----------------------------------------------------------------------------
};


#endif
