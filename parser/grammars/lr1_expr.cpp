/**
 * simple LR(1) expression test,
 * see lr1.cpp (simplified to have only + and * operators) for grammar and calculation of tables.
 *
 * @author Tobias Weber
 * @date 19-mar-20
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://en.wikipedia.org/wiki/Canonical_LR_parser
 *
 * g++ -std=c++17 -o lr1_expr lr1_expr.cpp lex.cpp
 */

#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <stack>
#include <tuple>
#include <algorithm>

using t_real = double;



// ----------------------------------------------------------------------------
// Symbols
// ----------------------------------------------------------------------------

enum class SymbolType
{
	TERM,
	NONTERM
};


/**
 * symbol base class
 */
class Symbol
{
public:
	Symbol(int id, t_real val=0, bool bEps=false, bool bEnd=false)
		: m_id{id}, m_val{val}, m_iseps{bEps}, m_isend{false} {}
	Symbol() = delete;

	virtual SymbolType GetType() const = 0;
	virtual std::shared_ptr<Symbol> clone() const = 0;

	int GetId() const { return m_id; }

	bool IsEps() const { return m_iseps; }
	bool IsEnd() const { return m_isend; }

	t_real GetVal() const { return m_val; }
	void SetVal(t_real d) { m_val = d; }


private:
	int m_id = -1;
	bool m_iseps = false;
	bool m_isend = false;

	// attribute
	t_real m_val = 0;
};



/**
 * terminal symbols
 */
class Terminal : public Symbol
{
public:
	Terminal(int id, t_real val=0, bool bEps=false, bool bEnd=false)
		: Symbol{id, val, bEps, bEnd} {}
	Terminal() = delete;

	virtual SymbolType GetType() const override
	{
		return SymbolType::TERM;
	}

	virtual std::shared_ptr<Symbol> clone() const override
	{
		return std::make_shared<Terminal>(GetId(), GetVal(), IsEps(), IsEnd());
	}
};


std::shared_ptr<Terminal> g_eps, g_end;


/**
 * nonterminal symbols
 */
class NonTerminal : public Symbol
{
public:
	NonTerminal(int id, t_real val=0) : Symbol{id, val} {}
	NonTerminal() = delete;

	virtual SymbolType GetType() const override
	{
		return SymbolType::NONTERM;
	}

	virtual std::shared_ptr<Symbol> clone() const override
	{
		return std::make_shared<NonTerminal>(GetId(), GetVal());
	}
};
// ----------------------------------------------------------------------------




// ----------------------------------------------------------------------------
// Lexer interface
// ----------------------------------------------------------------------------

#define TOK_REAL	1000
#define TOK_IDENT	1001
#define TOK_END		1002
#define TOK_EPS		1003

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
// Parser tables
// ----------------------------------------------------------------------------

#define NONTERM_ADD_TERM	2000
#define NONTERM_MUL_TERM	2001
#define NONTERM_FACTOR		2002

// (state, non-term symbol) -> state
std::map<std::tuple<int, int>, int> g_mapJump
{{
	{{0, NONTERM_ADD_TERM}, 11}, {{0, NONTERM_MUL_TERM}, 10}, {{0, NONTERM_FACTOR}, 3},
	{{2, NONTERM_ADD_TERM}, 4}, {{2, NONTERM_MUL_TERM}, 10}, {{2, NONTERM_FACTOR}, 3},
	{{5, NONTERM_MUL_TERM}, 6}, {{5, NONTERM_FACTOR}, 3},
	{{7, NONTERM_FACTOR}, 8},
}};

// (state, term symbol) -> state
std::map<std::tuple<int, int>, int> g_mapActionShift
{{
	{{0, TOK_REAL}, 1}, {{0, '('}, 2},
	{{2, TOK_REAL}, 1}, {{2, '('}, 2},
	{{4, '+'}, 5}, {{4, ')'}, 9},
	{{5, TOK_REAL}, 1}, {{5, '('}, 2},
	{{6, '*'}, 7},
	{{7, TOK_REAL}, 1}, {{7, '('}, 2},
	{{10, '*'}, 7},
	{{11, '+'}, 5},
}};

// (state, term symbol) -> production rule number
std::map<std::tuple<int, int>, int> g_mapActionReduce
{{
	{{1, '+'}, 6}, {{1, '*'}, 6}, {{1, ')'}, 6}, {{1, TOK_END}, 6},
	{{3, '+'}, 5}, {{3, '*'}, 5}, {{3, ')'}, 5}, {{3, TOK_END}, 5},
	{{6, '+'}, 2}, {{6, ')'}, 2}, {{6, TOK_END}, 2},
	{{8, '+'}, 4}, {{8, '*'}, 4}, {{8, ')'}, 4}, {{8, TOK_END}, 4},
	{{9, '+'}, 7}, {{9, '*'}, 7}, {{9, ')'}, 7}, {{9, TOK_END}, 7},
	{{10, '+'}, 3}, {{10, ')'}, 3}, {{10, TOK_END}, 3},
}};

// first symbol: lhs, further symbols: rhs
std::vector<std::shared_ptr<Symbol>> g_rules[] =
{
	/*0*/ {},

	/*1*/ {},	// accepting transition

	/*2*/ { std::make_shared<NonTerminal>(NONTERM_ADD_TERM),
			std::make_shared<NonTerminal>(NONTERM_ADD_TERM),
			std::make_shared<Terminal>('+'),
			std::make_shared<NonTerminal>(NONTERM_MUL_TERM) },

	/*3*/ { std::make_shared<NonTerminal>(NONTERM_ADD_TERM),
			std::make_shared<NonTerminal>(NONTERM_MUL_TERM) },

	/*4*/ { std::make_shared<NonTerminal>(NONTERM_MUL_TERM),
			std::make_shared<NonTerminal>(NONTERM_MUL_TERM),
			std::make_shared<Terminal>('*'),
			std::make_shared<NonTerminal>(NONTERM_FACTOR) },

	/*5*/ { std::make_shared<NonTerminal>(NONTERM_MUL_TERM),
			std::make_shared<NonTerminal>(NONTERM_FACTOR) },

	/*6*/ { std::make_shared<NonTerminal>(NONTERM_FACTOR),
			std::make_shared<Terminal>(TOK_REAL) },

	/*7*/ { std::make_shared<NonTerminal>(NONTERM_FACTOR),
			std::make_shared<Terminal>('('),
			std::make_shared<NonTerminal>(NONTERM_ADD_TERM),
			std::make_shared<Terminal>(')') },
};
// ----------------------------------------------------------------------------



int main()
{
	g_eps = std::make_shared<Terminal>(TOK_EPS, true, false);
	g_end = std::make_shared<Terminal>(TOK_END, false, true);


	std::cout << "Jump table" << std::endl;
	for(const auto &pair : g_mapJump)
	{
		std::cout << "(state " << std::get<0>(pair.first)
			<< ", non-terminal \"" << std::get<1>(pair.first) << "\") -> "
			<< "state " << pair.second << std::endl;
	}

	std::cout << "\nAction table (shifts)" << std::endl;
	for(const auto &pair : g_mapActionShift)
	{
		std::cout << "(state " << std::get<0>(pair.first)
			<< ", terminal " << std::get<1>(pair.first) << ") -> "
			<< "state " << pair.second << std::endl;
	}

	std::cout << "\nAction table (reductions)" << std::endl;
	for(const auto &pair : g_mapActionReduce)
	{
		std::cout << "(state " << std::get<0>(pair.first)
			<< ", terminal " << std::get<1>(pair.first) << ") -> "
			<< "rule " << pair.second << std::endl;
	}

	std::cout << std::endl;


	// stacks
	std::stack<int> states;
	std::stack<std::shared_ptr<Symbol>> symbols;

	states.push(0);		// starting state

	bool inv_op = 0;	// invert the operator?
	int tok = next_lookahead();
	t_real lval = yylval;
	if(tok == '*') { inv_op = 0; lval=1; }
	if(tok == '+') { inv_op = 0; lval=1; }
	if(tok == '/') { inv_op = 1; tok = '*'; lval=-1; }
	if(tok == '-') { inv_op = 1; tok = '+'; lval=-1; }

	while(1)
	{
		int topstate = states.top();

		auto iterActionShift = g_mapActionShift.find(std::make_tuple(topstate, tok));
		auto iterActionReduce = g_mapActionReduce.find(std::make_tuple(topstate, tok));

		// accepting state
		if(topstate == 11 && tok == TOK_END)
		{
			std::cout << "Accepting input. Top attribute: "
				<< symbols.top()->GetVal() << "." << std::endl;
			break;
		}

		if(iterActionShift != g_mapActionShift.end() && iterActionReduce != g_mapActionReduce.end())
		{
			std::cerr << "Error: Encountered shift-reduce conflict.";
			std::cerr << " Stack top state: " << topstate
				<< ", token: " << tok << " (" << char(tok) << ")"  << "." << std::endl;
			break;
		}
		if(iterActionShift == g_mapActionShift.end() && iterActionReduce == g_mapActionReduce.end())
		{
			std::cerr << "Error: Neither shift nor reduce action defined.";
			std::cerr << " Stack top state: " << topstate
				<< ", token: " << tok << " (" << char(tok) << ")"  << "." << std::endl;
			break;
		}


		// shift
		if(iterActionShift != g_mapActionShift.end())
		{
			int state = iterActionShift->second;
			states.push(state);
			symbols.push(std::make_shared<Terminal>(tok, lval));

			std::cout << "*** Shifting state " << state << " and token "
				<< tok << " (" << char(tok) << ") with value "
				<< symbols.top()->GetVal() << "." << std::endl;

			tok = next_lookahead();
			lval = yylval;
			if(tok == '*') { inv_op = 0; lval=1; }
			if(tok == '+') { inv_op = 0; lval=1; }
			if(tok == '/') { inv_op = 1; tok = '*'; lval=-1; }
			if(tok == '-') { inv_op = 1; tok = '+'; lval=-1; }
		}

		// reduce
		if(iterActionReduce != g_mapActionReduce.end())
		{
			const int rule = iterActionReduce->second;
			const auto& prod = g_rules[rule];

			std::cout << "*** Reducing with rule " << rule;
			std::cout << ", production has " << prod.size()-1 << " rhs symbols." << std::endl;

			std::vector<std::shared_ptr<Symbol>> rhs;
			for(std::size_t rhsidx=1; rhsidx < prod.size(); ++rhsidx)
			{
				rhs.push_back(symbols.top());

				states.pop();
				symbols.pop();
			}

			std::reverse(rhs.begin(), rhs.end());

			topstate = states.top();


			// push new symbol on top of stack
			auto lhs = prod[0]->clone();
			symbols.push(lhs);


			// execute semantic rules
			{
				if(rule == 2)
				{
					if(rhs[1]->GetVal() >= 0)
						lhs->SetVal(rhs[0]->GetVal() + rhs[2]->GetVal());
					else	// inverted operator
						lhs->SetVal(rhs[0]->GetVal() - rhs[2]->GetVal());
				}
				else if(rule == 4)
				{
					if(rhs[1]->GetVal() >= 0)
						lhs->SetVal(rhs[0]->GetVal() * rhs[2]->GetVal());
					else	// inverted operator
						lhs->SetVal(rhs[0]->GetVal() / rhs[2]->GetVal());
				}
				else if(rule == 3 || rule == 5 || rule == 6)
					lhs->SetVal(rhs[0]->GetVal());
				else if(rule == 7)
					lhs->SetVal(rhs[1]->GetVal());

				std::cout << "lhs attribute = " << lhs->GetVal() << std::endl;
			}


			auto iterJump = g_mapJump.find(std::make_tuple(topstate, lhs->GetId()));
			if(iterJump == g_mapJump.end())
			{
				std::cerr << "Error: Could not find jump table entry.";
				std::cerr << " Stack top state: " << topstate
					<< ", token: " << tok << " (" << char(tok) << ")"  << "." << std::endl;
				break;
			}

			// push new state on top of stack
			int state = iterJump->second;
			states.push(state);
		}
	}

	return 0;
}
