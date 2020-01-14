/**
 * parser test - syntax tree
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __AST_H__
#define __AST_H__

#include <memory>
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
class ASTCall;
class ASTAssign;


/**
 * ast visitor
 */
class ASTVisitor
{
public:
	virtual void visit(const ASTUMinus* ast) = 0;
	virtual void visit(const ASTPlus* ast) = 0;
	virtual void visit(const ASTMinus* ast) = 0;
	virtual void visit(const ASTMult* ast) = 0;
	virtual void visit(const ASTDiv* ast) = 0;
	virtual void visit(const ASTMod* ast) = 0;
	virtual void visit(const ASTPow* ast) = 0;
	virtual void visit(const ASTConst* ast) = 0;
	virtual void visit(const ASTVar* ast) = 0;
	virtual void visit(const ASTCall* ast) = 0;
	virtual void visit(const ASTAssign* ast) = 0;
};


#define ASTVISITOR_ACCEPT virtual void accept(ASTVisitor* visitor) const override { visitor->visit(this); }


/**
 * ast node base
 */
class AST
{
public:
	virtual void accept(ASTVisitor* visitor) const = 0;
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


class ASTCall : public AST
{
public:
	ASTCall(const std::string& ident, std::shared_ptr<AST> arg)
		: ident(ident), arg1(arg)
	{}

	ASTCall(const std::string& ident, std::shared_ptr<AST> arg1, std::shared_ptr<AST> arg2)
		: ident(ident), arg1(arg1), arg2(arg2)
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
