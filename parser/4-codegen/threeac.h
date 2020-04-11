/**
 * parser test - generate three-address code
 * @author Tobias Weber
 * @date 11-apr-20
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __THREEAC_H__
#define __THREEAC_H__

#include "ast.h"


class ThreeAC : public ASTVisitor
{
protected:
	std::string get_tmp_var()
	{
		std::string var{"t_"};
		var += std::to_string(m_varCount);
		++m_varCount;
		return var;
	}


public:
	virtual t_astret visit(const ASTUMinus* ast) override
	{
		t_astret term = ast->GetTerm()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = UMIN " << term << "\n";
		return var;
	}


	virtual t_astret visit(const ASTPlus* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = ADD " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTMinus* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = SUB " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTMult* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = MUL " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTDiv* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = DIV " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTMod* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = MOD " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTPow* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = POW " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTConst* ast) override
	{
		return std::to_string(ast->GetVal());
	}


	virtual t_astret visit(const ASTVar* ast) override
	{
		return ast->GetIdent();
	}


	virtual t_astret visit(const ASTCall* ast) override
	{
		std::vector<t_astret> params;

		if(ast->GetArg2())
			params.push_back(ast->GetArg2()->accept(this));
		if(ast->GetArg1())
			params.push_back(ast->GetArg1()->accept(this));

		for(const t_astret& param : params)
			(*m_ostr) << "CALLPARAM " << param << "\n";

		std::string var = get_tmp_var();

		(*m_ostr) << var << " = CALL " << ast->GetIdent()
			<< " " << params.size() << "\n";

		return var;
	}


	virtual t_astret visit(const ASTAssign* ast) override
	{
		t_astret expr = ast->GetExpr()->accept(this);
		std::string var = ast->GetIdent();

		(*m_ostr) << var << " = " << expr << "\n";
		return var;
	}


private:
	std::ostream* m_ostr = &std::cout;

	std::size_t m_varCount = 0;	// # of tmp vars
};


#endif
