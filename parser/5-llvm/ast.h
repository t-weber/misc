/**
 * parser test - syntax tree
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>


class AST;
class ASTUMinus;
class ASTPlus;
class ASTMinus;
class ASTMult;
class ASTDiv;
class ASTMod;
class ASTPow;
class ASTConst;
class ASTVar;
class ASTStmts;
class ASTArgs;
class ASTFunc;
class ASTCall;
class ASTAssign;


using t_astret = std::string;


/**
 * ast visitor
 */
class ASTVisitor
{
public:
	virtual ~ASTVisitor() {}

	virtual t_astret visit(const ASTUMinus* ast) = 0;
	virtual t_astret visit(const ASTPlus* ast) = 0;
	virtual t_astret visit(const ASTMinus* ast) = 0;
	virtual t_astret visit(const ASTMult* ast) = 0;
	virtual t_astret visit(const ASTDiv* ast) = 0;
	virtual t_astret visit(const ASTMod* ast) = 0;
	virtual t_astret visit(const ASTPow* ast) = 0;
	virtual t_astret visit(const ASTConst* ast) = 0;
	virtual t_astret visit(const ASTVar* ast) = 0;
	virtual t_astret visit(const ASTStmts* ast) = 0;
	virtual t_astret visit(const ASTArgs* ast) = 0;
	virtual t_astret visit(const ASTFunc* ast) = 0;
	virtual t_astret visit(const ASTCall* ast) = 0;
	virtual t_astret visit(const ASTAssign* ast) = 0;
};


#define ASTVISITOR_ACCEPT virtual t_astret accept(ASTVisitor* visitor) const override { return visitor->visit(this); }


/**
 * ast node base
 */
class AST
{
public:
	virtual t_astret accept(ASTVisitor* visitor) const = 0;
	virtual ~AST() {}
};


class ASTUMinus : public AST
{
public:
	ASTUMinus(std::shared_ptr<AST> term)
	: term(term)
	{}

	std::shared_ptr<AST> GetTerm() const { return term; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term;
};


class ASTPlus : public AST
{
public:
	ASTPlus(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTMinus : public AST
{
public:
	ASTMinus(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTMult : public AST
{
public:
	ASTMult(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTDiv : public AST
{
public:
	ASTDiv(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTMod : public AST
{
public:
	ASTMod(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTPow : public AST
{
public:
	ASTPow(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTConst : public AST
{
public:
	ASTConst(double val)
		: val(val)
	{}

	double GetVal() const { return val; }

	ASTVISITOR_ACCEPT

private:
	double val{};
};


class ASTVar : public AST
{
public:
	ASTVar(const std::string& ident)
		: ident(ident)
	{}

	const std::string& GetIdent() const { return ident; }

	ASTVISITOR_ACCEPT

private:
	std::string ident;
};


class ASTStmts : public AST
{
public:
	ASTStmts() : m_stmts{}
	{}

	void AddStatement(std::shared_ptr<AST> stmt)
	{
		m_stmts.push_back(stmt);
	}

	const std::list<std::shared_ptr<AST>>& GetStatementList() const
	{
		return m_stmts;
	}

	ASTVISITOR_ACCEPT

private:
	std::list<std::shared_ptr<AST>> m_stmts;
};


class ASTArgs : public AST
{
public:
	ASTArgs() : argnames{}
	{}

	void AddArg(const std::string& argname) { argnames.push_back(argname); }
	const std::vector<std::string>& GetArgs() const { return argnames; }

	ASTVISITOR_ACCEPT

private:
	std::vector<std::string> argnames;
};


class ASTFunc : public AST
{
public:
	ASTFunc(const std::string& ident, std::shared_ptr<ASTArgs>& args, std::shared_ptr<ASTStmts> stmts)
		: ident(ident), argnames(args->GetArgs()), stmts(stmts)
	{
		std::reverse(argnames.begin(), argnames.end());
	}

	const std::string& GetIdent() const { return ident; }
	const std::vector<std::string>& GetArgNames() const { return argnames; }
	std::shared_ptr<ASTStmts> GetStatements() const { return stmts; }

	ASTVISITOR_ACCEPT

private:
	std::string ident;
	std::vector<std::string> argnames;
	std::shared_ptr<ASTStmts> stmts;
};


class ASTCall : public AST
{
public:
	ASTCall(const std::string& ident, std::shared_ptr<AST> arg)
		: ident{ident}, arg1{arg}, arg2{nullptr}
	{}

	ASTCall(const std::string& ident, std::shared_ptr<AST> arg1, std::shared_ptr<AST> arg2)
		: ident{ident}, arg1{arg1}, arg2{arg2}
	{}

	const std::string& GetIdent() const { return ident; }
	std::shared_ptr<AST> GetArg1() const { return arg1; }
	std::shared_ptr<AST> GetArg2() const { return arg2; }

	ASTVISITOR_ACCEPT

private:
	std::string ident;
	std::shared_ptr<AST> arg1, arg2;
};


class ASTAssign : public AST
{
public:
	ASTAssign(const std::string& ident, std::shared_ptr<AST> expr)
		: ident(ident), expr(expr)
	{}

	const std::string& GetIdent() const { return ident; }
	std::shared_ptr<AST> GetExpr() const { return expr; }

	ASTVISITOR_ACCEPT

private:
	std::string ident;
	std::shared_ptr<AST> expr;
};



#endif
