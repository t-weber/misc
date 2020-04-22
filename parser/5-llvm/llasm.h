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
#include "sym.h"


class LLAsm : public ASTVisitor
{
protected:
	t_astret get_tmp_var(SymbolType ty = SymbolType::SCALAR,
		const std::array<unsigned int, 2>* dims = nullptr,
		const std::string* name = nullptr)
	{
		std::string var;
		if(name)
			var = *name;

		// create a unique temporary name
		if(!name || var == "")
		{
			var = "__tmp_";
			var += std::to_string(m_varCount);
			++m_varCount;
		}

		if(dims)
			return m_syms->AddSymbol(var, var, ty, *dims, true);
		else
			return m_syms->AddSymbol(var, var, ty, {0,0}, true);
	}


	std::string get_label()
	{
		std::string lab{"__lab_"};
		lab += std::to_string(m_labelCount);
		++m_labelCount;
		return lab;
	}


	/**
	 * find the symbol with a specific name in the symbol table
	 */
	t_astret get_sym(const std::string& name) const
	{
		const Symbol* sym = nullptr;
		if(m_syms)
			sym = m_syms->FindSymbol(name);

		if(sym==nullptr)
			std::cerr << "Error: \"" << name << "\" does not have an associated symbol." << std::endl;
		return sym;
	}


	/**
	 * ty1 + ty2 -> type
	 */
	SymbolType get_combined_type_plus(const Symbol* sym1, const Symbol* sym2)
	{
		SymbolType ty1 = sym1->ty;
		SymbolType ty2 = sym2->ty;

		if(ty1 == SymbolType::SCALAR && ty2 == SymbolType::SCALAR)
			return SymbolType::SCALAR;
		if(ty1 == SymbolType::SCALAR && ty2 == SymbolType::VECTOR)
			throw std::runtime_error{"Invalid operation."};
		if(ty1 == SymbolType::SCALAR && ty2 == SymbolType::MATRIX)
			throw std::runtime_error{"Invalid operation."};
		if(ty1 == SymbolType::SCALAR && ty2 == SymbolType::STRING)
				return SymbolType::STRING;

		if(ty1 == SymbolType::VECTOR && ty2 == SymbolType::SCALAR)
			throw std::runtime_error{"Invalid operation."};
		if(ty1 == SymbolType::VECTOR && ty2 == SymbolType::VECTOR)
			return SymbolType::VECTOR;
		if(ty1 == SymbolType::VECTOR && ty2 == SymbolType::MATRIX)
			throw std::runtime_error{"Invalid operation."};
		if(ty1 == SymbolType::VECTOR && ty2 == SymbolType::STRING)
			return SymbolType::STRING;

		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::SCALAR)
			throw std::runtime_error{"Invalid operation."};
		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::VECTOR)
			throw std::runtime_error{"Invalid operation."};
		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::MATRIX)
			return SymbolType::MATRIX;
		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::STRING)
			return SymbolType::STRING;

		if(ty1 == SymbolType::STRING && ty2 == SymbolType::SCALAR)
			return SymbolType::STRING;
		if(ty1 == SymbolType::STRING && ty2 == SymbolType::VECTOR)
			return SymbolType::STRING;
		if(ty1 == SymbolType::STRING && ty2 == SymbolType::MATRIX)
			return SymbolType::STRING;
		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::STRING)
			return SymbolType::STRING;

		throw std::runtime_error{"Invalid operation."};
	}


	/**
	 * ty1 * ty2 -> type
	 */
	SymbolType get_combined_type_mult(const Symbol* sym1, const Symbol* sym2)
	{
		SymbolType ty1 = sym1->ty;
		SymbolType ty2 = sym2->ty;

		if(ty1 == SymbolType::SCALAR && ty2 == SymbolType::SCALAR)
			return SymbolType::SCALAR;
		if(ty1 == SymbolType::SCALAR && ty2 == SymbolType::VECTOR)
			return SymbolType::VECTOR;
		if(ty1 == SymbolType::SCALAR && ty2 == SymbolType::MATRIX)
			return SymbolType::MATRIX;
		if(ty1 == SymbolType::SCALAR && ty2 == SymbolType::STRING)
			return SymbolType::STRING;

		if(ty1 == SymbolType::VECTOR && ty2 == SymbolType::SCALAR)
			return SymbolType::VECTOR;
		if(ty1 == SymbolType::VECTOR && ty2 == SymbolType::VECTOR)
			return SymbolType::SCALAR;
		if(ty1 == SymbolType::VECTOR && ty2 == SymbolType::MATRIX)
			return SymbolType::VECTOR;
		if(ty1 == SymbolType::VECTOR && ty2 == SymbolType::STRING)
			throw std::runtime_error{"Invalid operation."};

		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::SCALAR)
			return SymbolType::MATRIX;
		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::VECTOR)
			return SymbolType::VECTOR;
		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::MATRIX)
			return SymbolType::MATRIX;
		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::STRING)
			throw std::runtime_error{"Invalid operation."};

		if(ty1 == SymbolType::STRING && ty2 == SymbolType::SCALAR)
			return SymbolType::STRING;
		if(ty1 == SymbolType::STRING && ty2 == SymbolType::VECTOR)
			throw std::runtime_error{"Invalid operation."};
		if(ty1 == SymbolType::STRING && ty2 == SymbolType::MATRIX)
			throw std::runtime_error{"Invalid operation."};
		if(ty1 == SymbolType::MATRIX && ty2 == SymbolType::STRING)
			throw std::runtime_error{"Invalid operation."};

		throw std::runtime_error{"Invalid operation."};
	}


	/**
	 * get the corresponding data type name
	 */
	std::string get_type_name(SymbolType ty)
	{
		if(ty == SymbolType::SCALAR)
			return "double";
		else if(ty == SymbolType::VECTOR)
			return "double*";
		else if(ty == SymbolType::MATRIX)
			return "double*";
		else if(ty == SymbolType::STRING)
			return "i8*";

		std::cerr << "Error: Unknown symbol type." << std::endl;
		return "invalid";
	}


public:
	LLAsm(SymTab* syms) : m_syms{syms}
	{}


	virtual t_astret visit(const ASTUMinus* ast) override
	{
		t_astret term = ast->GetTerm()->accept(this);

		t_astret var = get_tmp_var(term->ty, &term->dims);
		(*m_ostr) << "%" << var->name << " = fneg double %" << term->name << "\n";
		return var;
	}


	virtual t_astret visit(const ASTPlus* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		SymbolType ty = get_combined_type_plus(term1, term2);
		t_astret var = get_tmp_var(ty, &term1->dims);

		std::string op = ast->IsInverted() ? "fsub" : "fadd";
		(*m_ostr) << "%" << var->name << " = " << op << " "
			<< get_type_name(ty) << " %" << term1->name << ", %" << term2->name << "\n";

		return var;
	}


	virtual t_astret visit(const ASTMult* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		SymbolType ty = get_combined_type_mult(term1, term2);
		t_astret var = get_tmp_var(ty, &term1->dims);

		std::string op = ast->IsInverted() ? "fdiv" : "fmul";
		(*m_ostr) << "%" << var->name << " = " << op << " "
			<< get_type_name(ty) << " %" << term1->name << ", %" << term2->name << "\n";

		return var;
	}


	virtual t_astret visit(const ASTMod* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		t_astret var = get_tmp_var(term1->ty, &term1->dims);

		(*m_ostr) << "%" << var->name << " = frem "
			<< get_type_name(term1->ty) << " %" << term1->name << ", %" << term2->name << "\n";
		return var;
	}


	virtual t_astret visit(const ASTPow* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);
		t_astret var = get_tmp_var(term1->ty, &term1->dims);

		(*m_ostr) << "%" << var->name << " = call double @pow("
			<< get_type_name(term1->ty) << " %" << term1->name << ", "
			<< get_type_name(term2->ty) << " %" << term2->name << ")\n";
		return var;
	}


	virtual t_astret visit(const ASTRealConst* ast) override
	{
		double val = ast->GetVal();

		// TODO: find a better way to assign a temporary variable
		t_astret retvar = get_tmp_var(SymbolType::SCALAR);
		t_astret retvar2 = get_tmp_var(SymbolType::SCALAR);
		(*m_ostr) << "%" << retvar->name << " = alloca double\n";
		(*m_ostr) << "store double " << std::scientific << val << ", double* %" << retvar->name << "\n";
		(*m_ostr) << "%" << retvar2->name << " = load double, double* %" << retvar->name << "\n";

		return retvar2;
	}


	virtual t_astret visit(const ASTVar* ast) override
	{
		t_astret sym = get_sym(ast->GetIdent());
		std::string var = std::string{"%"} + sym->name;

		if(sym->ty == SymbolType::SCALAR)
		{
			t_astret retvar = get_tmp_var(sym->ty, &sym->dims);
			(*m_ostr) << "%" << retvar->name << " = load double, double* " << var << "\n";
			return retvar;
		}

		// TODO: other types
		return nullptr;

	}


	virtual t_astret visit(const ASTArgNames*) override
	{
		return nullptr;
	}


	virtual t_astret visit(const ASTCall* ast) override
	{
		std::vector<t_astret> args;

		for(const auto& arg : ast->GetArgumentList())
			args.push_back(arg->accept(this));

		// TODO: other return types
		t_astret var = get_tmp_var(SymbolType::SCALAR);

		(*m_ostr) << "%" << var->name << " = call double @" << ast->GetIdent() << "(";
		for(std::size_t idx=0; idx<args.size(); ++idx)
		{
			(*m_ostr) << get_type_name(args[idx]->ty) << " %" << args[idx]->name;
			if(idx < args.size()-1)
				(*m_ostr) << ", ";
		}
		(*m_ostr) << ")\n";
		return var;
	}


	virtual t_astret visit(const ASTStmts* ast) override
	{
		t_astret lastres = nullptr;

		for(const auto& stmt : ast->GetStatementList())
			lastres = stmt->accept(this);

		return lastres;
	}


	virtual t_astret visit(const ASTVarDecl* ast) override
	{
		for(const auto& _var : ast->GetVariables())
		{
			t_astret sym = get_sym(_var);

			if(sym->ty == SymbolType::SCALAR)
				(*m_ostr) << "%" << sym->name << " = alloca double\n";

			// TODO: other types
		}

		return nullptr;
	}


	virtual t_astret visit(const ASTArgs*) override
	{
		return nullptr;
	}


	virtual t_astret visit(const ASTTypeDecl*) override
	{
		return nullptr;
	}


	virtual t_astret visit(const ASTFunc* ast) override
	{
		// TODO: other return and arg types

		(*m_ostr) << "define double @" << ast->GetIdent() << "(";

		auto argnames = ast->GetArgNames();
		for(std::size_t idx=0; idx<argnames.size(); ++idx)
		{
			const std::string arg = std::string{"f_"} + argnames[idx].first;
			(*m_ostr) << "double %" << arg;
			if(idx < argnames.size()-1)
				(*m_ostr) << ", ";
		}
		(*m_ostr) << ")\n{\n";


		// create local copies of the arguments
		for(std::size_t idx=0; idx<argnames.size(); ++idx)
		{
			// TODO: argument types
			const std::string arg = std::string{"f_"} + argnames[idx].first;
			t_astret symcpy = get_tmp_var(SymbolType::SCALAR, nullptr, &argnames[idx].first);

			(*m_ostr) << "%" << symcpy->name << " = alloca double\n";
			(*m_ostr) << "store double %" << arg << ", double* %" << symcpy->name << "\n";
		}


		t_astret lastres = ast->GetStatements()->accept(this);

		// return result of last expression
		if(lastres)
			(*m_ostr) << "ret double %" << lastres->name << "\n";
		else
			(*m_ostr) << "ret double 0." << lastres << "\n";

		(*m_ostr) << "}\n";

		return nullptr;
	}


	virtual t_astret visit(const ASTReturn* ast) override
	{
		if(ast->GetTerm())
		{
			t_astret term = ast->GetTerm()->accept(this);
			(*m_ostr) << "ret " << get_type_name(term->ty) << " %" << term->name << "\n";
		}
		else
		{
			(*m_ostr) << "ret double 0.\n";
		}

		return nullptr;
	}


	virtual t_astret visit(const ASTAssign* ast) override
	{
		t_astret expr = ast->GetExpr()->accept(this);
		std::string var = ast->GetIdent();

		if(expr->ty == SymbolType::SCALAR)
		{
			(*m_ostr) << "store " << get_type_name(expr->ty) << " %" << expr->name
				<< ", double* %" << var << "\n";

			// return a r-value if the variable is further needed
			t_astret retvar = get_tmp_var(expr->ty, &expr->dims);
			(*m_ostr) << "%" << retvar->name << " = load "
				<< get_type_name(expr->ty) << ", double* %" << var << "\n";

			return retvar;
		}

		// TODO: other types
		return nullptr;
	}


	virtual t_astret visit(const ASTComp* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);

		if(term1->ty != SymbolType::SCALAR || term2->ty != SymbolType::SCALAR)
			throw std::runtime_error{"Only scalar values can be compared."};

		t_astret var = get_tmp_var(term1->ty, &term1->dims);
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

		(*m_ostr) << "%" << var->name << " = fcmp " << op << " "
			<< get_type_name(term1->ty) << " %" << term1->name << ", %" << term2->name << "\n";
		return var;
	}


	virtual t_astret visit(const ASTCond* ast) override
	{
		t_astret cond = ast->GetCond()->accept(this);

		std::string labelIf = get_label();
		std::string labelElse = ast->HasElse() ? get_label() : "";
		std::string labelEnd = get_label();

		if(ast->HasElse())
			(*m_ostr) << "br i1 %" << cond->name << ", label %" << labelIf << ", label %" << labelElse << "\n";
		else
			(*m_ostr) << "br i1 %" << cond->name << ", label %" << labelIf << ", label %" << labelEnd << "\n";

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

		return nullptr;
	}


	virtual t_astret visit(const ASTLoop* ast) override
	{
		std::string labelStart = get_label();
		std::string labelBegin = get_label();
		std::string labelEnd = get_label();

		(*m_ostr) << "br label %" << labelStart << "\n";
		(*m_ostr) << labelStart << ":  ; loop start\n";
		t_astret cond = ast->GetCond()->accept(this);
		(*m_ostr) << "br i1 %" << cond->name << ", label %" << labelBegin << ", label %" << labelEnd << "\n";

		(*m_ostr) << labelBegin << ":  ; loop begin\n";
		ast->GetLoopStmt()->accept(this);
		(*m_ostr) << "br label %" << labelStart << "\n";
		(*m_ostr) << labelEnd << ":  ; loop end\n";
		return nullptr;
	}


private:
	std::ostream* m_ostr = &std::cout;

	std::size_t m_varCount = 0;	// # of tmp vars
	std::size_t m_labelCount = 0;	// # of labels

	SymTab* m_syms = nullptr;
};


#endif
