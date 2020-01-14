/**
 * parser test - generate zero-address code
 * @author Tobias Weber
 * @date 20-dec-19
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __ZEROAC_H__
#define __ZEROAC_H__

#include "ast.h"


class ZeroAC : public ASTVisitor
{
public:
	virtual void visit(const ASTUMinus* ast) override
	{
		ast->GetTerm()->accept(this);
		(*m_ostr) << "UMIN\n";
	}


	virtual void visit(const ASTPlus* ast) override
	{
		ast->GetTerm1()->accept(this);
		ast->GetTerm2()->accept(this);
		(*m_ostr) << "ADD\n";
	}


	virtual void visit(const ASTMinus* ast) override
	{
		ast->GetTerm1()->accept(this);
		ast->GetTerm2()->accept(this);
		(*m_ostr) << "SUB\n";
	}


	virtual void visit(const ASTMult* ast) override
	{
		ast->GetTerm1()->accept(this);
		ast->GetTerm2()->accept(this);
		(*m_ostr) << "MUL\n";
	}


	virtual void visit(const ASTDiv* ast) override
	{
		ast->GetTerm1()->accept(this);
		ast->GetTerm2()->accept(this);
		(*m_ostr) << "DIV\n";
	}


	virtual void visit(const ASTMod* ast) override
	{
		ast->GetTerm1()->accept(this);
		ast->GetTerm2()->accept(this);
		(*m_ostr) << "MOD\n";
	}


	virtual void visit(const ASTPow* ast) override
	{
		ast->GetTerm1()->accept(this);
		ast->GetTerm2()->accept(this);
		(*m_ostr) << "POW\n";
	}


	virtual void visit(const ASTConst* ast) override
	{
		(*m_ostr) << "PUSH " << ast->GetVal() << "\n";
	}


	virtual void visit(const ASTVar* ast) override
	{
		(*m_ostr) << "PUSHVAR " << ast->GetIdent() << "\n";
	}


	virtual void visit(const ASTCall* ast) override
	{
		std::size_t numArgs = 0;

		if(ast->GetArg2())
		{
			ast->GetArg2()->accept(this);
			++numArgs;
		}

		if(ast->GetArg1())
		{
			ast->GetArg1()->accept(this);
			++numArgs;
		}

		(*m_ostr) << "CALL " << ast->GetIdent() << " " << numArgs << "\n";
	}


	virtual void visit(const ASTAssign* ast) override
	{
		ast->GetExpr()->accept(this);
		(*m_ostr) << "ADDR " << ast->GetIdent() << "\n";
		(*m_ostr) << "ASSIGN\n";
	}


private:
	std::ostream* m_ostr = &std::cout;
};


#endif
