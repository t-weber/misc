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
class ASTMult;
class ASTMod;
class ASTPow;
class ASTTransp;
class ASTNorm;
class ASTStrConst;
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
class ASTArrayAssign;
class ASTArrayAccess;
class ASTComp;
class ASTCond;
class ASTLoop;
template<class> class ASTNumConst;
template<class> class ASTNumList;


enum class ASTType
{
	UMinus,
	Plus,
	Mult,
	Mod,
	Pow,
	Transp,
	Norm,
	StrConst,
	Var,
	Stmts,
	VarDecl,
	ArgNames,
	TypeDecl,
	Func,
	Return,
	Args,
	Call,
	Assign,
	ArrayAssign,
	ArrayAccess,
	Comp,
	Cond,
	Loop,
	NumConst,
	NumList
};


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
	virtual t_astret visit(const ASTTransp* ast) = 0;
	virtual t_astret visit(const ASTNorm* ast) = 0;
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
	virtual t_astret visit(const ASTArrayAssign* ast) = 0;
	virtual t_astret visit(const ASTArrayAccess* ast) = 0;
	virtual t_astret visit(const ASTComp* ast) = 0;
	virtual t_astret visit(const ASTCond* ast) = 0;
	virtual t_astret visit(const ASTLoop* ast) = 0;
	virtual t_astret visit(const ASTStrConst* ast) = 0;
	virtual t_astret visit(const ASTNumConst<double>* ast) = 0;
	virtual t_astret visit(const ASTNumConst<std::int64_t>* ast) = 0;
	virtual t_astret visit(const ASTNumList<double>* ast) = 0;
};


#define ASTVISITOR_ACCEPT virtual t_astret accept(ASTVisitor* visitor) const override { return visitor->visit(this); }


/**
 * ast node base
 */
class AST
{
public:
	virtual t_astret accept(ASTVisitor* visitor) const = 0;
	virtual ASTType type() = 0;

	virtual ~AST() {}
};


class ASTUMinus : public AST
{
public:
	ASTUMinus(std::shared_ptr<AST> term)
	: term{term}
	{}

	const std::shared_ptr<AST> GetTerm() const { return term; }

	virtual ASTType type() override { return ASTType::UMinus; }
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

	const std::shared_ptr<AST> GetTerm1() const { return term1; }
	const std::shared_ptr<AST> GetTerm2() const { return term2; }
	bool IsInverted() const { return inverted; }

	virtual ASTType type() override { return ASTType::Plus; }
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

	const std::shared_ptr<AST> GetTerm1() const { return term1; }
	const std::shared_ptr<AST> GetTerm2() const { return term2; }
	bool IsInverted() const { return inverted; }

	virtual ASTType type() override { return ASTType::Mult; }
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

	const std::shared_ptr<AST> GetTerm1() const { return term1; }
	const std::shared_ptr<AST> GetTerm2() const { return term2; }

	virtual ASTType type() override { return ASTType::Mod; }
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

	const std::shared_ptr<AST> GetTerm1() const { return term1; }
	const std::shared_ptr<AST> GetTerm2() const { return term2; }

	virtual ASTType type() override { return ASTType::Pow; }
	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTTransp : public AST
{
public:
	ASTTransp(std::shared_ptr<AST> term) : term{term}
	{}

	const std::shared_ptr<AST> GetTerm() const { return term; }

	virtual ASTType type() override { return ASTType::Transp; }
	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term;
};


class ASTNorm : public AST
{
public:
	ASTNorm(std::shared_ptr<AST> term) : term{term}
	{}

	const std::shared_ptr<AST> GetTerm() const { return term; }

	virtual ASTType type() override { return ASTType::Norm; }
	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term;
};


class ASTVar : public AST
{
public:
	ASTVar(const std::string& ident)
		: ident{ident}
	{}

	const std::string& GetIdent() const { return ident; }

	virtual ASTType type() override { return ASTType::Var; }
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

	virtual ASTType type() override { return ASTType::Stmts; }
	ASTVISITOR_ACCEPT

private:
	std::list<std::shared_ptr<AST>> stmts;
};


class ASTVarDecl : public AST
{
public:
	ASTVarDecl()
		: vars{}
	{}

	ASTVarDecl(std::shared_ptr<ASTAssign> optAssign)
		: vars{}, optAssign{optAssign}
	{}

	void AddVariable(const std::string& var)
	{
		vars.push_front(var);
	}

	const std::list<std::string>& GetVariables() const
	{
		return vars;
	}

	const std::shared_ptr<ASTAssign> GetAssignment() const { return optAssign; }

	virtual ASTType type() override { return ASTType::VarDecl; }
	ASTVISITOR_ACCEPT

private:
	std::list<std::string> vars;

	// optional assignment
	std::shared_ptr<ASTAssign> optAssign;
};


class ASTArgNames : public AST
{
public:
	ASTArgNames() : argnames{}
	{}

	void AddArg(const std::string& argname, SymbolType ty, std::size_t dim1=0, std::size_t dim2=0)
	{
		argnames.push_front(std::make_tuple(argname, ty, dim1, dim2));
	}

	const std::list<std::tuple<std::string, SymbolType, std::size_t, std::size_t>>& GetArgs() const
	{
		return argnames;
	}

	std::vector<SymbolType> GetArgTypes() const
	{
		std::vector<SymbolType> ty;
		for(const auto& arg : argnames)
			ty.push_back(std::get<1>(arg));
		return ty;
	}

	virtual ASTType type() override { return ASTType::ArgNames; }
	ASTVISITOR_ACCEPT

private:
	std::list<std::tuple<std::string, SymbolType, std::size_t, std::size_t>> argnames;
};


class ASTTypeDecl : public AST
{
public:
	ASTTypeDecl(SymbolType ty, std::size_t dim1=0, std::size_t dim2=0)
		: ty{ty}, dim1{dim1}, dim2{dim2}
	{}

	SymbolType GetType() const { return ty; }

	std::size_t GetDim(int i=0) const
	{
		if(i==0) return dim1;
		else if(i==1) return dim2;
		return 0;
	}

	std::tuple<SymbolType, std::size_t, std::size_t> GetRet() const
	{
		return std::make_tuple(ty, dim1, dim2);
	}

	virtual ASTType type() override { return ASTType::TypeDecl; }
	ASTVISITOR_ACCEPT

private:
	SymbolType ty;
	std::size_t dim1=0, dim2=0;
};


class ASTFunc : public AST
{
public:
	ASTFunc(const std::string& ident, std::shared_ptr<ASTTypeDecl>& rettype,
		std::shared_ptr<ASTArgNames>& args, std::shared_ptr<ASTStmts> stmts)
		: ident{ident}, rettype{rettype->GetRet()}, argnames{args->GetArgs()}, stmts{stmts}
	{}

	const std::string& GetIdent() const { return ident; }
	std::tuple<SymbolType, std::size_t, std::size_t> GetRetType() const { return rettype; }

	const std::list<std::tuple<std::string, SymbolType, std::size_t, std::size_t>>&
	GetArgNames() const { return argnames; }

	const std::shared_ptr<ASTStmts> GetStatements() const { return stmts; }

	virtual ASTType type() override { return ASTType::Func; }
	ASTVISITOR_ACCEPT

private:
	std::string ident;
	std::tuple<SymbolType, std::size_t, std::size_t> rettype;
	std::list<std::tuple<std::string, SymbolType, std::size_t, std::size_t>> argnames;
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

	const std::shared_ptr<AST> GetTerm() const { return term; }

	virtual ASTType type() override { return ASTType::Return; }
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

	virtual ASTType type() override { return ASTType::Args; }
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

	virtual ASTType type() override { return ASTType::Call; }
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
	const std::shared_ptr<AST> GetExpr() const { return expr; }

	virtual ASTType type() override { return ASTType::Assign; }
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

	const std::shared_ptr<AST> GetTerm1() const { return term1; }
	const std::shared_ptr<AST> GetTerm2() const { return term2; }
	CompOp GetOp() const { return op; }

	virtual ASTType type() override { return ASTType::Comp; }
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

	const std::shared_ptr<AST> GetCond() const { return cond; }
	const std::shared_ptr<AST> GetIf() const { return if_stmt; }
	const std::shared_ptr<AST> GetElse() const { return else_stmt; }
	bool HasElse() const { return else_stmt != nullptr; }

	virtual ASTType type() override { return ASTType::Cond; }
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

	const std::shared_ptr<AST> GetCond() const { return cond; }
	const std::shared_ptr<AST> GetLoopStmt() const { return stmt; }

	virtual ASTType type() override { return ASTType::Loop; }
	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> cond, stmt;
};


class ASTArrayAccess : public AST
{
public:
	ASTArrayAccess(std::shared_ptr<AST> term,
		std::shared_ptr<AST> num1, std::shared_ptr<AST> num2 = nullptr)
	: term{term}, num1{num1}, num2{num2}
	{}

	const std::shared_ptr<AST> GetTerm() const { return term; }
	const std::shared_ptr<AST> GetNum1() const { return num1; }
	const std::shared_ptr<AST> GetNum2() const { return num2; }

	virtual ASTType type() override { return ASTType::ArrayAccess; }
	ASTVISITOR_ACCEPT

private:
	std::shared_ptr<AST> term;
	std::shared_ptr<AST> num1, num2;
};


class ASTArrayAssign : public AST
{
public:
	ASTArrayAssign(const std::string& ident, std::shared_ptr<AST> expr,
		std::shared_ptr<AST> num1, std::shared_ptr<AST> num2 = nullptr
	)
		: ident{ident}, expr{expr}, num1{num1}, num2{num2}
	{}

	const std::string& GetIdent() const { return ident; }
	const std::shared_ptr<AST> GetExpr() const { return expr; }
	const std::shared_ptr<AST> GetNum1() const { return num1; }
	const std::shared_ptr<AST> GetNum2() const { return num2; }

	virtual ASTType type() override { return ASTType::ArrayAssign; }
	ASTVISITOR_ACCEPT

private:
	std::string ident;
	std::shared_ptr<AST> expr;
	std::shared_ptr<AST> num1, num2;
};


template<class t_num>
class ASTNumConst : public AST
{
public:
	ASTNumConst(t_num val) : val{val}
	{}

	t_num GetVal() const { return val; }

	virtual ASTType type() override { return ASTType::NumConst; }
	ASTVISITOR_ACCEPT

private:
	t_num val{};
};


class ASTStrConst : public AST
{
public:
	ASTStrConst(const std::string& str) : val{str}
	{}

	const std::string& GetVal() const { return val; }

	virtual ASTType type() override { return ASTType::StrConst; }
	ASTVISITOR_ACCEPT

private:
	std::string val;
};


template<class t_num = double>
class ASTNumList : public AST
{
public:
	ASTNumList()
	{}

	void AddNum(t_num num)
	{
		nums.push_front(num);
	}

	const std::list<t_num>& GetList() const
	{
		return nums;
	}

	virtual ASTType type() override { return ASTType::NumList; }
	ASTVISITOR_ACCEPT

private:
	std::list<t_num> nums;
};


#endif
