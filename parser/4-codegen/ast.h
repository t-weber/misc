/**
 * parser test
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __AST_H__
#define __AST_H__

#include <memory>
#include <iostream>


class AST
{
public:
	// generate 0-address code
	virtual void Generate0AC(std::ostream& ostr) const = 0;
};


class ASTUMinus : public AST
{
public:
	ASTUMinus(std::shared_ptr<AST> term)
	: term(term)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		term->Generate0AC(ostr);
		ostr << "UMIN\n";
	}

private:
	std::shared_ptr<AST> term;
};


class ASTPlus : public AST
{
public:
	ASTPlus(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		term1->Generate0AC(ostr);
		term2->Generate0AC(ostr);
		ostr << "ADD\n";
	}

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTMinus : public AST
{
public:
	ASTMinus(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		term1->Generate0AC(ostr);
		term2->Generate0AC(ostr);
		ostr << "SUB\n";
	}

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTMult : public AST
{
public:
	ASTMult(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		term1->Generate0AC(ostr);
		term2->Generate0AC(ostr);
		ostr << "MUL\n";
	}

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTDiv : public AST
{
public:
	ASTDiv(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		term1->Generate0AC(ostr);
		term2->Generate0AC(ostr);
		ostr << "DIV\n";
	}

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTMod : public AST
{
public:
	ASTMod(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		term1->Generate0AC(ostr);
		term2->Generate0AC(ostr);
		ostr << "MOD\n";
	}

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTPow : public AST
{
public:
	ASTPow(std::shared_ptr<AST> term1, std::shared_ptr<AST> term2)
		: term1(term1), term2(term2)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		term1->Generate0AC(ostr);
		term2->Generate0AC(ostr);
		ostr << "POW\n";
	}

private:
	std::shared_ptr<AST> term1, term2;
};


class ASTConst : public AST
{
public:
	ASTConst(double val)
		: val(val)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		ostr << "PUSH " << val << "\n";
	}

private:
	double val{};
};


class ASTVar : public AST
{
public:
	ASTVar(const std::string& ident)
		: ident(ident)
	{}

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		ostr << "PUSHVAR " << ident << "\n";
	}

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

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		std::size_t numArgs = 0;
		if(arg2)
		{
			arg2->Generate0AC(ostr);
			++numArgs;
		}
		if(arg1)
		{
			arg1->Generate0AC(ostr);
			++numArgs;
		}
		ostr << "CALL " << ident << " " << numArgs << "\n";
	}

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

	virtual void Generate0AC(std::ostream& ostr) const override
	{
		expr->Generate0AC(ostr);
		ostr << "ADDR " << ident << "\n";
		ostr << "ASSIGN\n";
	}

private:
	std::string ident;
	std::shared_ptr<AST> expr;
};


#endif
