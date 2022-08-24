/**
 * LR(1) expression parser via recursive ascent
 *
 * @author Tobias Weber
 * @date 21-aug-2022
 * @license see 'LICENSE.EUPL' file
 *
 * Reference for the algorithm:
 *    https://doi.org/10.1016/0020-0190(88)90061-0
 */

#ifndef __EXPR_PARSER_RECASC_H__
#define __EXPR_PARSER_RECASC_H__

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <variant>
#include <unordered_map>
#include <cmath>


using t_val = double;


struct Symbol
{
	bool is_expr{false};
	std::variant<t_val, std::string> val{};
};


struct Token
{
	enum : int
	{
		REAL    = 1000,
		IDENT   = 1001,
		END     = 1002,

		INVALID = 10000,
	};

	int id{INVALID};
	t_val val{};
	std::string strval{};
};


class ExprParser
{
public:
	ExprParser() = default;
	~ExprParser() = default;
	ExprParser(const ExprParser&) = delete;
	ExprParser& operator=(const ExprParser&) = delete;

	t_val Parse(const std::string& expr);


protected:
	// --------------------------------------------------------------------
	// lexer
	// --------------------------------------------------------------------
	// find all matching tokens for input string
	static std::vector<Token> get_matching_tokens(const std::string& str);

	// returns the next token
	static Token lex(std::istream* /*= &std::cin*/);

	void GetNextLookahead();
	// --------------------------------------------------------------------

	// --------------------------------------------------------------------
	t_val GetValue(const Symbol& sym) const;
	t_val GetIdentValue(const std::string& ident) const;
	Symbol CallFunc(const std::string& ident) const;
	Symbol CallFunc(const std::string& ident, const Symbol& arg) const;
	Symbol CallFunc(const std::string& ident, const Symbol& arg1, const Symbol& arg2) const;
	// --------------------------------------------------------------------

	// --------------------------------------------------------------------
	// LR closures
	// --------------------------------------------------------------------
	void start();
	void after_expr();

	void add_after_op();
	void after_add();

	void sub_after_op();
	void after_sub();

	void mul_after_op();
	void after_mul();

	void div_after_op();
	void after_div();

	void mod_after_op();
	void after_mod();

	void pow_after_op();
	void after_pow();

	void after_bracket();
	void bracket_after_expr();
	void after_bracket_expr();

	void after_ident();
	void after_real();

	void funccall_after_ident();
	void funccall_after_arg();
	void funccall_after_comma();
	void funccall_after_arg2();
	void after_funccall_0args();
	void after_funccall_1arg();
	void after_funccall_2args();

	void usub_after_op();
	void after_usub();

	void uadd_after_op();
	void after_uadd();
	// --------------------------------------------------------------------

	static void TransitionError(const char* func, int token);


private:
	std::shared_ptr<std::istream> m_istr{};

	Token m_lookahead{};
	std::stack<Symbol> m_symbols{};
	bool m_accepted{false};

	std::size_t m_dist_to_jump{0};

	// --------------------------------------------------------------------
	// Tables
	// --------------------------------------------------------------------
	// symbol table
	std::unordered_map<std::string, t_val> m_mapSymbols =
	{
		{ "pi", t_val(M_PI) },
	};

	// zero-args function table
	std::unordered_map<std::string, t_val(*)()> m_mapFuncs0 =
	{ };

	// one-arg function table
	std::unordered_map<std::string, t_val(*)(t_val)> m_mapFuncs1 =
	{
		{ "sin", [](t_val x) -> t_val { return (t_val)std::sin(x); } },
		{ "cos", [](t_val x) -> t_val { return (t_val)std::cos(x); } },
		{ "tan", [](t_val x) -> t_val { return (t_val)std::tan(x); } },

		{ "sqrt", [](t_val x) -> t_val { return (t_val)std::sqrt(x); } },
		{ "exp", [](t_val x) -> t_val { return (t_val)std::exp(x); } },
	};

	// two-args function table
	std::unordered_map<std::string, t_val(*)(t_val, t_val)> m_mapFuncs2 =
	{
		{ "pow", [](t_val x, t_val y) -> t_val { return (t_val)std::pow(x, y); } },
	};
	// ----------------------------------------------------------------------------
};

#endif
