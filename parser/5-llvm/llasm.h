/**
 * parser test - generate llvm three-address code
 * @author Tobias Weber
 * @date 11-apr-20
 * @license: see 'LICENSE.GPL' file
 *
 * References:
 *	* https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl03.html
 *	* https://llvm.org/docs/GettingStarted.html
 *	* https://llvm.org/docs/LangRef.html
 */

#ifndef __LLASM_H__
#define __LLASM_H__

#include "ast.h"


class LLAsm : public ASTVisitor
{
protected:
	std::string get_tmp_var()
	{
		std::string var{"%t_"};
		var += std::to_string(m_varCount);
		++m_varCount;
		return var;
	}


public:
	virtual t_astret visit(const ASTUMinus* ast) override
	{
		t_astret term = ast->GetTerm()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = fneg double " << term << "\n";
		return var;
	}


	virtual t_astret visit(const ASTPlus* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = fadd double " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTMinus* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = fsub double " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTMult* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = fmul double " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTDiv* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = fdiv double " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTMod* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = frem double " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTPow* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();
		(*m_ostr) << var << " = fpow double " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTConst* ast) override
	{
		return std::to_string(ast->GetVal());
	}


	virtual t_astret visit(const ASTVar* ast) override
	{
		std::string var = std::string{"%"} + ast->GetIdent();
		return var;
	}


	virtual t_astret visit(const ASTCall* ast) override
	{
		std::vector<t_astret> args;

		if(ast->GetArg1())
			args.push_back(ast->GetArg1()->accept(this));
		if(ast->GetArg2())
			args.push_back(ast->GetArg2()->accept(this));

		std::string var = get_tmp_var();

		(*m_ostr) << var << " = call double @" << ast->GetIdent() << "(";
		for(std::size_t idx=0; idx<args.size(); ++idx)
		{
			(*m_ostr) << "double " << args[idx];
			if(idx < args.size()-1)
				(*m_ostr) << ", ";
		}
		(*m_ostr) << ")\n";
		return var;
	}


	virtual t_astret visit(const ASTStmts* ast) override
	{
		t_astret lastres;

		auto stmts = ast->GetStatementList();
		for(auto iter=stmts.rbegin(); iter!=stmts.rend(); ++iter)
			lastres = (*iter)->accept(this);

		return lastres;
	}


	virtual t_astret visit(const ASTArgs*) override
	{
		return t_astret{};
	}


	virtual t_astret visit(const ASTFunc* ast) override
	{
		(*m_ostr) << "define double @" << ast->GetIdent() << "(";

		auto argnames = ast->GetArgNames();
		for(std::size_t idx=0; idx<argnames.size(); ++idx)
		{
			const std::string& arg = argnames[idx];
			(*m_ostr) << "double %" << arg;
			if(idx < argnames.size()-1)
				(*m_ostr) << ", ";
		}
		(*m_ostr) << ")\n{\nentry:\n";

		// return result of last expression
		t_astret lastres = ast->GetStatements()->accept(this);
		(*m_ostr) << "ret double " << lastres << "\n";

		(*m_ostr) << "}\n";

		return t_astret{};
	}


	virtual t_astret visit(const ASTAssign* ast) override
	{
		t_astret expr = ast->GetExpr()->accept(this);
		std::string var = std::string{"%"} + ast->GetIdent();

		(*m_ostr) << var << " = " << expr << "\n";
		return var;
	}


private:
	std::ostream* m_ostr = &std::cout;

	std::size_t m_varCount = 0;	// # of tmp vars
};


#endif
