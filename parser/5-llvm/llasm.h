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

	std::string get_label()
	{
		std::string lab{"l_"};
		lab += std::to_string(m_labelCount);
		++m_labelCount;
		return lab;
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

		std::string op = ast->IsInverted() ? "fsub" : "fadd";
		(*m_ostr) << var << " = " << op << " double " << term1 << ", " << term2 << "\n";

		return var;
	}


	virtual t_astret visit(const ASTMult* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		std::string var = get_tmp_var();

		std::string op = ast->IsInverted() ? "fdiv" : "fmul";
		(*m_ostr) << var << " = " << op << " double " << term1 << ", " << term2 << "\n";

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
		(*m_ostr) << var << " = call double @pow(double " << term1 << ", double " << term2 << ")\n";
		return var;
	}


	virtual t_astret visit(const ASTRealConst* ast) override
	{
		return std::to_string(ast->GetVal());
	}


	virtual t_astret visit(const ASTVar* ast) override
	{
		std::string var = std::string{"%"} + ast->GetIdent();

		std::string retvar = get_tmp_var();
		(*m_ostr) << retvar << " = load double, double* " << var << "\n";

		return retvar;
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


	virtual t_astret visit(const ASTVarDecl* ast) override
	{
		auto vars = ast->GetVariables();
		for(auto iter=vars.rbegin(); iter!=vars.rend(); ++iter)
		{
			const std::string var = std::string{"%"} + *iter;

			(*m_ostr) << var << " = alloca double\n";
		}

		return t_astret{};
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
			const std::string arg = std::string{"f_"} + argnames[idx];
			(*m_ostr) << "double %" << arg;
			if(idx < argnames.size()-1)
				(*m_ostr) << ", ";
		}
		(*m_ostr) << ")\n{\n";


		// create local copies of the arguments
		for(std::size_t idx=0; idx<argnames.size(); ++idx)
		{
			const std::string arg = std::string{"f_"} + argnames[idx];
			const std::string& argcpy = argnames[idx];

			(*m_ostr) << "%" << argcpy << " = alloca double\n";
			(*m_ostr) << "store double %" << arg << ", double* %" << argcpy << "\n";
		}


		t_astret lastres = ast->GetStatements()->accept(this);

		// return result of last expression
		if(lastres == "") lastres = "0.";
		(*m_ostr) << "ret double " << lastres << "\n";

		(*m_ostr) << "}\n";

		return t_astret{};
	}


	virtual t_astret visit(const ASTReturn* ast) override
	{
		if(ast->GetTerm())
		{
			t_astret term = ast->GetTerm()->accept(this);
			(*m_ostr) << "ret double " << term << "\n";
		}
		else
		{
			(*m_ostr) << "ret double 0.\n";
		}

		return t_astret{};
	}


	virtual t_astret visit(const ASTAssign* ast) override
	{
		t_astret expr = ast->GetExpr()->accept(this);
		std::string var = std::string{"%"} + ast->GetIdent();
		(*m_ostr) << "store double " << expr << ", double* " << var << "\n";

		// return a r-value if the variable is further needed
		std::string retvar = get_tmp_var();
		(*m_ostr) << retvar << " = load double, double* " << var << "\n";
		return retvar;
	}


	virtual t_astret visit(const ASTComp* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);

		std::string var = get_tmp_var();
		std::string op;
		switch(ast->GetOp())
		{
			case ASTComp::EQU: op = "oeq"; break;
			case ASTComp::NEQ: op = "one"; break;
			case ASTComp::GT: op = "ogt"; break;
			case ASTComp::LT: op = "olt"; break;
			case ASTComp::GEQ: op = "oge"; break;
			case ASTComp::LEQ: op = "ole"; break;
		}

		(*m_ostr) << var << " = fcmp " << op << " double " << term1 << ", " << term2 << "\n";
		return var;
	}


	virtual t_astret visit(const ASTCond* ast) override
	{
		t_astret cond = ast->GetCond()->accept(this);

		std::string labelIf = get_label();
		std::string labelElse = ast->HasElse() ? get_label() : "";
		std::string labelEnd = get_label();

		if(ast->HasElse())
			(*m_ostr) << "br i1 " << cond << ", label %" << labelIf << ", label %" << labelElse << "\n";
		else
			(*m_ostr) << "br i1 " << cond << ", label %" << labelIf << ", label %" << labelEnd << "\n";

		(*m_ostr) << labelIf << ":  ; if branch\n";
		ast->GetIf()->accept(this);
		(*m_ostr) << "br label %" << labelEnd << "\n";

		if(ast->HasElse())
		{
			(*m_ostr) << labelElse << ":  ; else branch\n";
			ast->GetElse()->accept(this);
			(*m_ostr) << "br label %" << labelEnd << "\n";
		}

		(*m_ostr) << labelEnd << ":  ; endif\n";

		return t_astret{};
	}


	virtual t_astret visit(const ASTLoop* ast) override
	{
		std::string labelStart = get_label();
		std::string labelBegin = get_label();
		std::string labelEnd = get_label();

		(*m_ostr) << "br label %" << labelStart << "\n";
		(*m_ostr) << labelStart << ":  ; loop start\n";
		t_astret cond = ast->GetCond()->accept(this);
		(*m_ostr) << "br i1 " << cond << ", label %" << labelBegin << ", label %" << labelEnd << "\n";

		(*m_ostr) << labelBegin << ":  ; loop begin\n";
		ast->GetLoopStmt()->accept(this);
		(*m_ostr) << "br label %" << labelStart << "\n";
		(*m_ostr) << labelEnd << ":  ; loop end\n";
		return t_astret{};
	}


private:
	std::ostream* m_ostr = &std::cout;

	std::size_t m_varCount = 0;	// # of tmp vars
	std::size_t m_labelCount = 0;	// # of labels
};


#endif
