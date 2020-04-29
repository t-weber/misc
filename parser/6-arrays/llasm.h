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
		const std::array<std::size_t, 2>* dims = nullptr,
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
		std::string scoped_name;
		for(const std::string& scope : m_curscope)
			scoped_name += scope + "::";	// scope name separator
		scoped_name += name;

		const Symbol* sym = nullptr;
		if(m_syms)
		{
			sym = m_syms->FindSymbol(scoped_name);

			// try global scope instead
			if(!sym)
				sym = m_syms->FindSymbol(name);
		}

		if(sym==nullptr)
			std::cerr << "Error: \"" << scoped_name << "\" does not have an associated symbol." << std::endl;
		return sym;
	}


	/**
	 * convert symbol to another type
	 */
	t_astret convert_sym(t_astret sym, SymbolType ty_to)
	{
		// already the correct type
		if(sym->ty == ty_to)
			return sym;

		std::string op;
		if(sym->ty == SymbolType::INT and ty_to == SymbolType::SCALAR)
			op = "sitofp";
		else if(sym->ty == SymbolType::SCALAR and ty_to == SymbolType::INT)
			op = "fptosi";

		if(op == "")
			throw std::runtime_error("Invalid type conversion.");

		std::string from = get_type_name(sym->ty);
		std::string to = get_type_name(ty_to);

		t_astret var = get_tmp_var(ty_to, &sym->dims);
		(*m_ostr) << "%" << var->name << " = " << op << " " << from << "%" << sym->name << " to " << to << "\n";

		return var;
	}


	/**
	 * get the corresponding data type name
	 */
	std::string get_type_name(SymbolType ty)
	{
		switch(ty)
		{
			case SymbolType::SCALAR: return "double";
			case SymbolType::VECTOR: return "double*";
			case SymbolType::MATRIX: return "double*";
			case SymbolType::STRING: return "i8*";
			case SymbolType::INT: return "i64";
			case SymbolType::VOID: return "void";
		}

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

		if(term->ty == SymbolType::SCALAR)
		{
			(*m_ostr) << "%" << var->name << " = fneg " << get_type_name(term->ty)
				<< " %" << term->name << "\n";
		}
		else if(term->ty == SymbolType::INT)
		{
			(*m_ostr) << "%" << var->name << " = sub " << get_type_name(term->ty) << " "
				<< "0, %" << term->name << "\n";
		}
		return var;
	}


	virtual t_astret visit(const ASTPlus* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);

		// cast if needed
		SymbolType ty = term1->ty;
		if(term1->ty==SymbolType::SCALAR || term2->ty==SymbolType::SCALAR)
			ty = SymbolType::SCALAR;
		t_astret var = get_tmp_var(ty, &term1->dims);

		term1 = convert_sym(term1, ty);
		term2 = convert_sym(term2, ty);


		std::string op = ast->IsInverted() ? "sub" : "add";
		if(ty == SymbolType::SCALAR)
			op = "f" + op;

		(*m_ostr) << "%" << var->name << " = " << op << " "
			<< get_type_name(ty) << " %" << term1->name << ", %" << term2->name << "\n";

		return var;
	}


	virtual t_astret visit(const ASTMult* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);

		// cast if needed
		SymbolType ty = term1->ty;
		if(term1->ty==SymbolType::SCALAR || term2->ty==SymbolType::SCALAR)
			ty = SymbolType::SCALAR;
		t_astret var = get_tmp_var(ty, &term1->dims);

		term1 = convert_sym(term1, ty);
		term2 = convert_sym(term2, ty);


		std::string op = ast->IsInverted() ? "div" : "mul";
		if(ty == SymbolType::SCALAR)
			op = "f" + op;
		else if(ty == SymbolType::INT && ast->IsInverted())
			op = "s" + op;

		(*m_ostr) << "%" << var->name << " = " << op << " "
			<< get_type_name(ty) << " %" << term1->name << ", %" << term2->name << "\n";

		return var;
	}


	virtual t_astret visit(const ASTMod* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);

		// cast if needed
		SymbolType ty = term1->ty;
		if(term1->ty==SymbolType::SCALAR || term2->ty==SymbolType::SCALAR)
			ty = SymbolType::SCALAR;
		t_astret var = get_tmp_var(ty, &term1->dims);

		term1 = convert_sym(term1, ty);
		term2 = convert_sym(term2, ty);


		std::string op = "rem";
		if(ty == SymbolType::SCALAR)
			op = "f" + op;
		else if(ty == SymbolType::INT)
			op = "s" + op;

		(*m_ostr) << "%" << var->name << " = " << op << " "
			<< get_type_name(ty) << " %" << term1->name << ", %" << term2->name << "\n";
		return var;
	}


	virtual t_astret visit(const ASTPow* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);

		// cast if needed
		SymbolType ty = term1->ty;
		if(term1->ty==SymbolType::SCALAR || term2->ty==SymbolType::SCALAR)
			ty = SymbolType::SCALAR;
		t_astret var = get_tmp_var(ty, &term1->dims);

		term1 = convert_sym(term1, ty);
		term2 = convert_sym(term2, ty);


		(*m_ostr) << "%" << var->name << " = call double @pow("
			<< get_type_name(ty) << " %" << term1->name << ", "
			<< get_type_name(ty) << " %" << term2->name << ")\n";
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


	virtual t_astret visit(const ASTIntConst* ast) override
	{
		std::int64_t val = ast->GetVal();

		// TODO: find a better way to assign a temporary variable
		t_astret retvar = get_tmp_var(SymbolType::INT);
		t_astret retvar2 = get_tmp_var(SymbolType::INT);
		(*m_ostr) << "%" << retvar->name << " = alloca i64\n";
		(*m_ostr) << "store i64 " << val << ", i64* %" << retvar->name << "\n";
		(*m_ostr) << "%" << retvar2->name << " = load i64, i64* %" << retvar->name << "\n";

		return retvar2;
	}


	virtual t_astret visit(const ASTStrConst* ast) override
	{
		const std::string& str = ast->GetVal();
		std::size_t dim = str.length()+1;

		std::array<std::size_t, 2> dims{{dim, 0}};
		t_astret str_mem = get_tmp_var(SymbolType::STRING, &dims);

		// allocate the string's memory
		(*m_ostr) << "%" << str_mem->name << " = alloca [" << dim << " x i8]\n";

		for(std::size_t idx=0; idx<dim; ++idx)
		{
			t_astret ptr = get_tmp_var();
			(*m_ostr) << "%" << ptr->name << " = getelementptr [" << dim << " x i8], ["
				<< dim << " x i8]* %" << str_mem->name << ", i64 0, i64 " << idx << "\n";

			int val = (idx<dim-1) ? str[idx] : 0;
			(*m_ostr) << "store i8 " << val << ", i8* %"  << ptr->name << "\n";
		}

		return str_mem;
	}


	virtual t_astret visit(const ASTVar* ast) override
	{
		t_astret sym = get_sym(ast->GetIdent());
		if(sym == nullptr)
			throw std::runtime_error("Symbol \"" + ast->GetIdent() + "\" not in symbol table.");

		std::string var = std::string{"%"} + sym->name;

		if(sym->ty == SymbolType::SCALAR || sym->ty == SymbolType::INT)
		{
			t_astret retvar = get_tmp_var(sym->ty, &sym->dims);
			std::string ty = get_type_name(sym->ty);
			(*m_ostr) << "%" << retvar->name << " = load " << ty  << ", " << ty << "* " << var << "\n";
			return retvar;
		}
		else if(sym->ty == SymbolType::VECTOR || sym->ty == SymbolType::MATRIX)
		{
			return sym;
		}
		else if(sym->ty == SymbolType::STRING)
		{
			return sym;
		}
		else
		{
			throw std::runtime_error("Invalid type for visited variable: \"" + sym->name + "\".");
		}

		return nullptr;
	}


	virtual t_astret visit(const ASTCall* ast) override
	{
		const std::string& funcname = ast->GetIdent();
		t_astret func = get_sym(funcname);
		if(func == nullptr)
			throw std::runtime_error("Function \"" + funcname + "\" not in symbol table.");
		if(ast->GetArgumentList().size() != func->argty.size())
			throw std::runtime_error("Invalid number of function parameters for \"" + funcname + "\".");

		std::vector<t_astret> args;

		std::size_t _idx=0;
		for(const auto& curarg : ast->GetArgumentList())
		{
			t_astret arg = curarg->accept(this);

			// cast if needed
			t_astret arg_casted = convert_sym(arg, func->argty[_idx]);
			if(arg_casted->ty == SymbolType::STRING)
			{
				// string arguments are of type i8*, so use a pointer to the string's array
				t_astret strptr = get_tmp_var(arg_casted->ty, &arg_casted->dims);

				(*m_ostr) << "%" << strptr->name << " = getelementptr ["
					<< std::get<0>(arg_casted->dims) << " x i8], ["
					<< std::get<0>(arg_casted->dims) << " x i8]* %"
					<< arg_casted->name << ", i64 0, i64 0\n";

				args.push_back(strptr);
			}
			else
			{
				args.push_back(arg_casted);
			}

			++_idx;
		}

		// TODO: other return types
		t_astret retvar = get_tmp_var(func->retty);
		std::string retty = get_type_name(func->retty);

		if(func->retty != SymbolType::VOID)
			(*m_ostr) << "%" << retvar->name << " = ";
		(*m_ostr) << "call " << retty << " @" << funcname << "(";
		for(std::size_t idx=0; idx<args.size(); ++idx)
		{
			(*m_ostr) << get_type_name(args[idx]->ty) << " %" << args[idx]->name;
			if(idx < args.size()-1)
				(*m_ostr) << ", ";
		}
		(*m_ostr) << ")\n";
		return retvar;
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
			std::string ty = get_type_name(sym->ty);

			if(sym->ty == SymbolType::SCALAR || sym->ty == SymbolType::INT)
			{
				(*m_ostr) << "%" << sym->name << " = alloca " << ty << "\n";
			}
			else if(sym->ty == SymbolType::VECTOR || sym->ty == SymbolType::MATRIX)
			{
				std::size_t dim = std::get<0>(sym->dims);
				if(sym->ty == SymbolType::MATRIX)
					dim *= std::get<1>(sym->dims);

				// allocate the array's memory
				(*m_ostr) << "%" << sym->name << " = alloca [" << dim << " x double]\n";
			}
			else if(sym->ty == SymbolType::STRING)
			{
				std::size_t dim = std::get<0>(sym->dims);

				// allocate the string's memory
				(*m_ostr) << "%" << sym->name << " = alloca [" << dim << " x i8]\n";

				// get a pointer to the string
				t_astret strptr = get_tmp_var();
				(*m_ostr) << "%" << strptr->name << " = getelementptr [" << dim << " x i8], ["
					<< dim << " x i8]* %" << sym->name << ", i64 0, i64 0\n";

				// set first element to zero
				(*m_ostr) << "store i8 0, i8* %"  << strptr->name << "\n";
			}
			else
			{
				throw std::runtime_error("Invalid type in declaration: \"" + sym->name + "\".");
			}
		}

		return nullptr;
	}


	virtual t_astret visit(const ASTFunc* ast) override
	{
		m_curscope.push_back(ast->GetIdent());

		std::string rettype = get_type_name(ast->GetRetType());
		(*m_ostr) << "define " << rettype << " @" << ast->GetIdent() << "(";

		auto argnames = ast->GetArgNames();
		std::size_t idx=0;
		for(const auto& [argname, argtype] : argnames)
		{
			const std::string arg = std::string{"f_"} + argname;
			(*m_ostr) << get_type_name(argtype) << " %" << arg;
			if(idx < argnames.size()-1)
				(*m_ostr) << ", ";
			++idx;
		}
		(*m_ostr) << ")\n{\n";


		// create local copies of the arguments
		for(const auto& [argname, argtype] : argnames)
		{
			if(argtype == SymbolType::SCALAR || argtype == SymbolType::INT)
			{
				const std::string arg = std::string{"f_"} + argname;
				t_astret symcpy = get_tmp_var(argtype, nullptr, &argname);

				std::string ty = get_type_name(argtype);
				(*m_ostr) << "%" << symcpy->name << " = alloca " << ty << "\n";
				(*m_ostr) << "store " << ty << " %" << arg << ", " << ty << "* %" << symcpy->name << "\n";
			}

			// TODO: other argument types
		}


		t_astret lastres = ast->GetStatements()->accept(this);

		if(ast->GetRetType() == SymbolType::VOID)
		{
			(*m_ostr) << "ret void\n";
		}
		else
		{
			// return result of last expression
			if(lastres)
				(*m_ostr) << "ret " << rettype << " %" << lastres->name << "\n";
			else
				(*m_ostr) << "ret " << rettype << " 0" << "\n";
		}

		(*m_ostr) << "}\n";
		m_curscope.pop_back();
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
			(*m_ostr) << "ret void\n";
		}

		return nullptr;
	}


	virtual t_astret visit(const ASTAssign* ast) override
	{
		t_astret expr = ast->GetExpr()->accept(this);
		std::string var = ast->GetIdent();
		t_astret sym = get_sym(var);

		// cast if needed
		if(expr->ty != sym->ty)
			expr = convert_sym(expr, sym->ty);

		if(expr->ty == SymbolType::SCALAR || expr->ty == SymbolType::INT)
		{
			std::string ty = get_type_name(expr->ty);
			(*m_ostr) << "store " << ty << " %" << expr->name << ", "<< ty << "* %" << var << "\n";
		}
		else if(sym->ty == SymbolType::VECTOR || sym->ty == SymbolType::MATRIX)
		{
			if(std::get<0>(expr->dims) != std::get<0>(sym->dims))
				throw std::runtime_error("Dimension mismatch in assignment of \"" + sym->name + "\".");

			std::size_t dim = std::get<0>(expr->dims);
			if(expr->ty == SymbolType::MATRIX)
			{
				if(std::get<1>(expr->dims) != std::get<1>(sym->dims))
					throw std::runtime_error("Dimension mismatch in assignment of \"" + sym->name + "\".");

				dim *= std::get<1>(expr->dims);
			}

			// copy elements in a loop
			std::string labelStart = get_label();
			std::string labelBegin = get_label();
			std::string labelEnd = get_label();

			// loop counter
			t_astret ctr = get_tmp_var(SymbolType::INT);
			(*m_ostr) << "%" << ctr->name << " = alloca i64\n";
			(*m_ostr) << "store i64 0, i64* %" << ctr->name << "\n";

			(*m_ostr) << "br label %" << labelStart << "\n";
			(*m_ostr) << labelStart << ":  ; loop start\n";

			// loop condition: ctr < dim
			t_astret ctrval = get_tmp_var(SymbolType::INT);
			(*m_ostr) << "%" << ctrval->name << " = load i64, i64* %" << ctr->name << "\n";

			t_astret cond = get_tmp_var();
			(*m_ostr) << "%" << cond->name << " = icmp slt i64 %" << ctrval->name <<  ", " << dim << "\n";
			(*m_ostr) << "br i1 %" << cond->name << ", label %" << labelBegin << ", label %" << labelEnd << "\n";

			(*m_ostr) << labelBegin << ":  ; loop begin\n";

			// ---------------
			// loop statements
			t_astret elemptr_src = get_tmp_var(SymbolType::STRING);
			(*m_ostr) << "%" << elemptr_src->name << " = getelementptr [" << dim << " x double], ["
				<< dim << " x double]* %" << expr->name << ", i64 0, i64 %" << ctrval->name << "\n";
			t_astret elemptr_dst = get_tmp_var(SymbolType::STRING);
			(*m_ostr) << "%" << elemptr_dst->name << " = getelementptr [" << dim << " x double], ["
				<< dim << " x double]* %" << sym->name << ", i64 0, i64 %" << ctrval->name << "\n";
			t_astret elem_src = get_tmp_var(SymbolType::STRING);
			(*m_ostr) << "%" << elem_src->name << " = load double, double* %" << elemptr_src->name << "\n";

			(*m_ostr) << "store double %" << elem_src->name << ", double* %" << elemptr_dst->name << "\n";

			// increment counter
			t_astret newctrval = get_tmp_var(SymbolType::INT);
			(*m_ostr) << "%" << newctrval->name << " = add i64 %" << ctrval->name << ", 1\n";
			(*m_ostr) << "store i64 %" << newctrval->name << ", i64* %" << ctr->name << "\n";
			// ---------------

			(*m_ostr) << "br label %" << labelStart << "\n";
			(*m_ostr) << labelEnd << ":  ; loop end\n";
		}
		else if(sym->ty == SymbolType::STRING)
		{
			std::size_t src_dim = std::get<0>(expr->dims);
			std::size_t dst_dim = std::get<0>(sym->dims);
			if(src_dim > dst_dim)
				throw std::runtime_error("Buffer of string \"" + sym->name + "\" is not large enough.");
			std::size_t dim = std::min(src_dim, dst_dim);

			// copy elements in a loop
			std::string labelStart = get_label();
			std::string labelBegin = get_label();
			std::string labelEnd = get_label();

			// loop counter
			t_astret ctr = get_tmp_var(SymbolType::INT);
			(*m_ostr) << "%" << ctr->name << " = alloca i64\n";
			(*m_ostr) << "store i64 0, i64* %" << ctr->name << "\n";

			(*m_ostr) << "br label %" << labelStart << "\n";
			(*m_ostr) << labelStart << ":  ; loop start\n";

			// loop condition: ctr < dim
			t_astret ctrval = get_tmp_var(SymbolType::INT);
			(*m_ostr) << "%" << ctrval->name << " = load i64, i64* %" << ctr->name << "\n";

			t_astret cond = get_tmp_var();
			(*m_ostr) << "%" << cond->name << " = icmp slt i64 %" << ctrval->name <<  ", " << dim << "\n";
			(*m_ostr) << "br i1 %" << cond->name << ", label %" << labelBegin << ", label %" << labelEnd << "\n";

			(*m_ostr) << labelBegin << ":  ; loop begin\n";

			// ---------------
			// loop statements
			t_astret elemptr_src = get_tmp_var(SymbolType::STRING);
			(*m_ostr) << "%" << elemptr_src->name << " = getelementptr [" << src_dim << " x i8], ["
				<< src_dim << " x i8]* %" << expr->name << ", i64 0, i64 %" << ctrval->name << "\n";
			t_astret elemptr_dst = get_tmp_var(SymbolType::STRING);
			(*m_ostr) << "%" << elemptr_dst->name << " = getelementptr [" << dst_dim << " x i8], ["
				<< dst_dim << " x i8]* %" << sym->name << ", i64 0, i64 %" << ctrval->name << "\n";
			t_astret elem_src = get_tmp_var(SymbolType::STRING);
				(*m_ostr) << "%" << elem_src->name << " = load i8, i8* %" << elemptr_src->name << "\n";

			(*m_ostr) << "store i8 %" << elem_src->name << ", i8* %" << elemptr_dst->name << "\n";

			// increment counter
			t_astret newctrval = get_tmp_var(SymbolType::INT);
			(*m_ostr) << "%" << newctrval->name << " = add i64 %" << ctrval->name << ", 1\n";
			(*m_ostr) << "store i64 %" << newctrval->name << ", i64* %" << ctr->name << "\n";
			// ---------------

			(*m_ostr) << "br label %" << labelStart << "\n";
			(*m_ostr) << labelEnd << ":  ; loop end\n";
		}

		return expr;
	}


	virtual t_astret visit(const ASTComp* ast) override
	{
		t_astret term1 = ast->GetTerm1()->accept(this);
		t_astret term2 = ast->GetTerm2()->accept(this);

		// cast if needed
		SymbolType ty = term1->ty;
		if(term1->ty==SymbolType::SCALAR || term2->ty==SymbolType::SCALAR)
			ty = SymbolType::SCALAR;
		t_astret var = get_tmp_var(ty, &term1->dims);

		term1 = convert_sym(term1, ty);
		term2 = convert_sym(term2, ty);


		std::string op;
		switch(ast->GetOp())
		{
			case ASTComp::EQU: op = "eq"; break;
			case ASTComp::NEQ: op = "ne"; break;
			case ASTComp::GT: op = "gt"; break;
			case ASTComp::LT: op = "lt"; break;
			case ASTComp::GEQ: op = "ge"; break;
			case ASTComp::LEQ: op = "le"; break;
		}

		std::string cmpop;
		switch(ty)
		{
			case SymbolType::SCALAR:
			{
				cmpop = "fcmp";
				op = "o" + op;
				break;
			}
			case SymbolType::INT:
			{
				cmpop = "icmp";
				if(op != "eq" && op != "ne")
					op = "s" + op;	// signed
				break;
			}
			// TODO: other types
		}

		(*m_ostr) << "%" << var->name << " = " << cmpop << " " << op << " "
			<< get_type_name(ty) << " %" << term1->name << ", %" << term2->name << "\n";
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


	// ------------------------------------------------------------------------
	// internally handled dummy nodes
	// ------------------------------------------------------------------------
	virtual t_astret visit(const ASTArgNames*) override { return nullptr; }
	virtual t_astret visit(const ASTArgs*) override { return nullptr; }
	virtual t_astret visit(const ASTTypeDecl*) override { return nullptr; }
	// ------------------------------------------------------------------------


private:
	std::ostream* m_ostr = &std::cout;

	std::size_t m_varCount = 0;	// # of tmp vars
	std::size_t m_labelCount = 0;	// # of labels

	std::vector<std::string> m_curscope;

	SymTab* m_syms = nullptr;
};


#endif
