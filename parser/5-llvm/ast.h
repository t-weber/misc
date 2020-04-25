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
#include <cstdint>

#include "sym.h"


class AST;
class ASTUMinus;
class ASTPlus;
class ASTMinus;
class ASTMult;
class ASTDiv;
class ASTMod;
class ASTPow;
class ASTRealConst;
class ASTIntConst;
class ASTVar;
class ASTStmts;
class ASTVarDecl;
class ASTArgNames;
class ASTTypeDecl;
class ASTFunc;
class ASTReturn;
class ASTArgs;
class ASTCall;
class ASTAssign;
class ASTComp;
class ASTCond;
class ASTLoop;


using t_astret = const Symbol*;


/**
 * ast visitor
 */
class ASTVisitor
{
public:
	virtual ~ASTVisitor() {}

	virtual t_astret visit(const ASTUMinus* ast) = 0;
	virtual t_astret visit(const ASTPlus* ast) = 0;
	virtual t_astret visit(const ASTMult* ast) = 0;
	virtual t_astret visit(const ASTMod* ast) = 0;
	virtual t_astret visit(const ASTPow* ast) = 0;
	virtual t_astret visit(const ASTRealConst* ast) = 0;
	virtual t_astret visit(const ASTIntConst* ast) = 0;
	virtual t_astret visit(const ASTVar* ast) = 0;
	virtual t_astret visit(const ASTStmts* ast) = 0;
	virtual t_astret visit(const ASTVarDecl* ast) = 0;
	virtual t_astret visit(const ASTArgNames* ast) = 0;
	virtual t_astret visit(const ASTTypeDecl* ast) = 0;
	virtual t_astret visit(const ASTFunc* ast) = 0;
	virtual t_astret visit(const ASTReturn* ast) = 0;
	virtual t_astret visit(const ASTArgs* ast) = 0;
	virtual t_astret visit(const ASTCall* ast) = 0;
	virtual t_astret visit(const ASTAssign* ast) = 0;
	virtual t_astret visit(const ASTComp* ast) = 0;
	virtual t_astret visit(const ASTCond* ast) = 0;
	virtual t_astret visit(const ASTLoop* ast) = 0;
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
	: term{term}
	{}

	std::shared_ptr<AST> GetTerm() const { return term; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term;
};


class ASTPlus : public AST
{
public:
	ASTPlus(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2,
		bool invert = 0)
		: term1{term1}, term2{term2}, inverted{invert}
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }
	bool IsInverted() const { return inverted; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
	bool inverted = 0;
};


class ASTMult : public AST
{
public:
	ASTMult(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2,
		bool invert = 0)
		: term1{term1}, term2{term2}, inverted{invert}
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }
	bool IsInverted() const { return inverted; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
	bool inverted = 0;
};


class ASTMod : public AST
{
public:
	ASTMod(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1{term1}, term2{term2}
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
		: term1{term1}, term2{term2}
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTRealConst : public AST
{
public:
	ASTRealConst(double val) : val{val}
	{}

	double GetVal() const { return val; }

	ASTVISITOR_ACCEPT

private:
	double val{};
};


class ASTIntConst : public AST
{
public:
	ASTIntConst(std::int64_t val) : val{val}
	{}

	std::int64_t GetVal() const { return val; }

	ASTVISITOR_ACCEPT

private:
	std::int64_t val{};
};


class ASTVar : public AST
{
public:
	ASTVar(const std::string& ident)
		: ident{ident}
	{}

	const std::string& GetIdent() const { return ident; }

	ASTVISITOR_ACCEPT

private:
	std::string ident;
};


class ASTStmts : public AST
{
public:
	ASTStmts() : stmts{}
	{}

	void AddStatement(std::shared_ptr<AST> stmt)
	{
		stmts.push_front(stmt);
	}

	const std::list<std::shared_ptr<AST>>& GetStatementList() const
	{
		return stmts;
	}

	ASTVISITOR_ACCEPT

private:
	std::list<std::shared_ptr<AST>> stmts;
};


class ASTVarDecl : public AST
{
public:
	ASTVarDecl() : vars{}
	{}

	void AddVariable(const std::string& var)
	{
		vars.push_front(var);
	}

	const std::list<std::string>& GetVariables() const
	{
		return vars;
	}

	ASTVISITOR_ACCEPT

private:
	std::list<std::string> vars;
};


class ASTArgNames : public AST
{
public:
	ASTArgNames() : argnames{}
	{}

	void AddArg(const std::string& argname, SymbolType ty)
	{
		argnames.push_back(std::make_pair(argname, ty));
	}

	const std::vector<std::pair<std::string, SymbolType>>& GetArgs() const
	{
		return argnames;
	}

	std::vector<SymbolType> GetArgTypes() const
	{
		std::vector<SymbolType> ty;
		for(const auto& arg : argnames)
			ty.push_back(arg.second);

		return ty;
	}

	ASTVISITOR_ACCEPT

private:
	std::vector<std::pair<std::string, SymbolType>> argnames;
};


class ASTTypeDecl : public AST
{
public:
	ASTTypeDecl(SymbolType ty) : ty{ty}
	{}

	SymbolType GetType() const { return ty; }

	ASTVISITOR_ACCEPT

private:
	SymbolType ty;
};


class ASTFunc : public AST
{
public:
	ASTFunc(const std::string& ident, std::shared_ptr<ASTTypeDecl>& rettype,
		std::shared_ptr<ASTArgNames>& args, std::shared_ptr<ASTStmts> stmts)
		: ident{ident}, rettype{rettype}, argnames{args->GetArgs()}, stmts{stmts}
	{
		std::reverse(argnames.begin(), argnames.end());
	}

	const std::string& GetIdent() const { return ident; }
	SymbolType GetRetType() const { return rettype->GetType(); }
	const std::vector<std::pair<std::string, SymbolType>>& GetArgNames() const { return argnames; }
	std::shared_ptr<ASTStmts> GetStatements() const { return stmts; }

	ASTVISITOR_ACCEPT

private:
	std::string ident;
	std::shared_ptr<ASTTypeDecl> rettype;
	std::vector<std::pair<std::string, SymbolType>> argnames;
	std::shared_ptr<ASTStmts> stmts;
};


class ASTReturn : public AST
{
public:
	ASTReturn(std::shared_ptr<AST> term)
		: term{term}
	{}
	ASTReturn()
	{}

	std::shared_ptr<AST> GetTerm() const { return term; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term;
};


class ASTArgs : public AST
{
public:
	ASTArgs() : args{}
	{}

	void AddArgument(std::shared_ptr<AST> arg)
	{
		args.push_front(arg);
	}

	const std::list<std::shared_ptr<AST>>& GetArgumentList() const
	{
		return args;
	}

	ASTVISITOR_ACCEPT

private:
	std::list<std::shared_ptr<AST>> args;
};


class ASTCall : public AST
{
public:
	ASTCall(const std::string& ident)
		: ident{ident}, args{std::make_shared<ASTArgs>()}
	{}

	ASTCall(const std::string& ident, std::shared_ptr<ASTArgs> args)
		: ident{ident}, args{args}
	{}

	const std::string& GetIdent() const { return ident; }
	const std::list<std::shared_ptr<AST>>& GetArgumentList() const { return args->GetArgumentList(); }

	ASTVISITOR_ACCEPT

private:
	std::string ident;
	std::shared_ptr<ASTArgs> args;
};


class ASTAssign : public AST
{
public:
	ASTAssign(const std::string& ident, std::shared_ptr<AST> expr)
		: ident{ident}, expr{expr}
	{}

	const std::string& GetIdent() const { return ident; }
	std::shared_ptr<AST> GetExpr() const { return expr; }

	ASTVISITOR_ACCEPT

private:
	std::string ident;
	std::shared_ptr<AST> expr;
};


class ASTComp : public AST
{
public:
	enum CompOp
	{
		EQU, NEQ,
		GT, LT, GEQ, LEQ
	};

public:
	ASTComp(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2, CompOp op)
		: term1{term1}, term2{term2}, op{op}
	{}

	ASTComp(std::shared_ptr<AST> term1, CompOp op)
		: term1{term1}, term2{nullptr}, op{op}
	{}

	std::shared_ptr<AST> GetTerm1() const { return term1; }
	std::shared_ptr<AST> GetTerm2() const { return term2; }
	CompOp GetOp() const { return op; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
	CompOp op;
};


class ASTCond : public AST
{
public:
	ASTCond(const std::shared_ptr<AST> cond, std::shared_ptr<AST> if_stmt)
		: cond{cond}, if_stmt{if_stmt}
	{}
	ASTCond(const std::shared_ptr<AST> cond, std::shared_ptr<AST> if_stmt, std::shared_ptr<AST> else_stmt)
		: cond{cond}, if_stmt{if_stmt}, else_stmt{else_stmt}
	{}

	std::shared_ptr<AST> GetCond() const { return cond; }
	std::shared_ptr<AST> GetIf() const { return if_stmt; }
	std::shared_ptr<AST> GetElse() const { return else_stmt; }
	bool HasElse() const { return else_stmt != nullptr; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> cond;
	std::shared_ptr<AST> if_stmt, else_stmt;
};


class ASTLoop : public AST
{
public:
	ASTLoop(const std::shared_ptr<AST> cond, std::shared_ptr<AST> stmt)
		: cond{cond}, stmt{stmt}
	{}

	std::shared_ptr<AST> GetCond() const { return cond; }
	std::shared_ptr<AST> GetLoopStmt() const { return stmt; }

	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> cond, stmt;
};


#endif
