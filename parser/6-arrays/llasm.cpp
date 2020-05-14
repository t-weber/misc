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

#include "llasm.h"


t_astret LLAsm::get_tmp_var(SymbolType ty, const std::array<std::size_t, 2>* dims, const std::string* name)
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


std::string LLAsm::get_label()
{
	std::string lab{"__lab_"};
	lab += std::to_string(m_labelCount);
	++m_labelCount;
	return lab;
}


/**
	* find the symbol with a specific name in the symbol table
	*/
t_astret LLAsm::get_sym(const std::string& name) const
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
t_astret LLAsm::convert_sym(t_astret sym, SymbolType ty_to)
{
	// already the correct type
	if(sym->ty == ty_to)
		return sym;

	// re-interpret vector as matrix
	else if(ty_to == SymbolType::MATRIX && sym->ty == SymbolType::VECTOR)
		return sym;


	// scalar conversions
	if(ty_to == SymbolType::SCALAR || ty_to == SymbolType::INT)
	{
		std::string op;
		if(sym->ty == SymbolType::INT && ty_to == SymbolType::SCALAR)
			op = "sitofp";
		else if(sym->ty == SymbolType::SCALAR && ty_to == SymbolType::INT)
			op = "fptosi";

		if(op == "")
			throw std::runtime_error("Invalid scalar type conversion.");

		std::string from = get_type_name(sym->ty);
		std::string to = get_type_name(ty_to);

		t_astret var = get_tmp_var(ty_to, &sym->dims);
		(*m_ostr) << "%" << var->name << " = " << op << " " << from << "%" << sym->name << " to " << to << "\n";
		return var;
	}

	// conversions to string
	else if(ty_to == SymbolType::STRING)
	{
		// ... from int
		if(sym->ty == SymbolType::INT)
		{
			std::size_t len = 32;
			std::array<std::size_t, 2> dims{{len, 0}};
			t_astret str_mem = get_tmp_var(SymbolType::STRING, &dims);
			t_astret strptr = get_tmp_var(SymbolType::STRING, &dims);

			(*m_ostr) << "%" << str_mem->name << " = alloca [" << len << " x i8]\n";
			(*m_ostr) << "%" << strptr->name << " = getelementptr ["
				<< len << " x i8], [" << len << " x i8]* %"
				<< str_mem->name << ", i64 0, i64 0\n";

			(*m_ostr) << "call void @int_to_str(i64 %"  << sym->name
				<< ", i8* %" << strptr->name << ", i64 " << len << ")\n";
			return str_mem;
		}

		// ... from (double) scalar
		else if(sym->ty == SymbolType::SCALAR)
		{
			std::size_t len = 32;
			std::array<std::size_t, 2> dims{{len, 0}};
			t_astret str_mem = get_tmp_var(SymbolType::STRING, &dims);
			t_astret strptr = get_tmp_var(SymbolType::STRING, &dims);

			(*m_ostr) << "%" << str_mem->name << " = alloca [" << len << " x i8]\n";
			(*m_ostr) << "%" << strptr->name << " = getelementptr ["
				<< len << " x i8], [" << len << " x i8]* %"
				<< str_mem->name << ", i64 0, i64 0\n";

			(*m_ostr) << "call void @flt_to_str(double %"  << sym->name
				<< ", i8* %" << strptr->name << ", i64 " << len << ")\n";
			return str_mem;
		}

		// ... from vector or matrix
		else if(sym->ty == SymbolType::VECTOR || sym->ty == SymbolType::MATRIX)
		{
			std::size_t num_floats = std::get<0>(sym->dims);
			if(sym->ty == SymbolType::MATRIX)
				num_floats *= std::get<1>(sym->dims);

			std::size_t len = 32 * num_floats;
			std::array<std::size_t, 2> dims{{len, 0}};
			t_astret str_mem = get_tmp_var(SymbolType::STRING, &dims);
			t_astret strptr = get_tmp_var(SymbolType::STRING, &dims);

			(*m_ostr) << "%" << str_mem->name << " = alloca [" << len << " x i8]\n";
			//(*m_ostr) << "%" << strptr->name << " = bitcast [" << len << " x i8]* %" << str_mem->name << " to i8*\n";
			(*m_ostr) << "%" << strptr->name << " = getelementptr ["
				<< len << " x i8], [" << len << " x i8]* %"
				<< str_mem->name << ", i64 0, i64 0\n";


			// prepare "[ ", "] ", ", ", and "; " strings
			t_astret vecbegin = get_tmp_var(SymbolType::STRING);
			t_astret vecend = get_tmp_var(SymbolType::STRING);
			t_astret vecsep = get_tmp_var(SymbolType::STRING);
			t_astret matsep = nullptr;

			(*m_ostr) << "%" << vecbegin->name << " = bitcast [3 x i8]* @__str_vecbegin to i8*\n";
			(*m_ostr) << "%" << vecend->name << " = bitcast [3 x i8]* @__str_vecend to i8*\n";
			(*m_ostr) << "%" << vecsep->name << " = bitcast [3 x i8]* @__str_vecsep to i8*\n";

			if(sym->ty == SymbolType::MATRIX)
			{
				matsep = get_tmp_var(SymbolType::STRING);
				(*m_ostr) << "%" << matsep->name << " = bitcast [3 x i8]* @__str_matsep to i8*\n";
			}


			// vector start: "[ "
			(*m_ostr) << "call i8* @strncpy(i8* %" << strptr->name << ", i8* %" << vecbegin->name << ", i64 3)\n";

			for(std::size_t i=0; i<num_floats; ++i)
			{
				// get vector/matrix element
				t_astret elemptr = get_tmp_var();
				t_astret elem = get_tmp_var();

				(*m_ostr) << "%" << elemptr->name << " = getelementptr [" << num_floats << " x double], ["
					<< num_floats << " x double]* %" << sym->name << ", i64 0, i64 " << i << "\n";
				(*m_ostr) << "%" << elem->name << " = load double, double* %" << elemptr->name << "\n";


				// convert vector/matrix component to string
				std::size_t lenComp = 32;
				std::array<std::size_t, 2> dimsComp{{lenComp, 0}};
				t_astret strComp_mem = get_tmp_var(SymbolType::STRING, &dimsComp);
				t_astret strCompptr = get_tmp_var(SymbolType::STRING, &dimsComp);

				(*m_ostr) << "%" << strComp_mem->name << " = alloca [" << lenComp << " x i8]\n";
				(*m_ostr) << "%" << strCompptr->name << " = getelementptr ["
					<< lenComp << " x i8], [" << lenComp << " x i8]* %"
					<< strComp_mem->name << ", i64 0, i64 0\n";

				(*m_ostr) << "call void @flt_to_str(double %"  << elem->name
					<< ", i8* %" << strCompptr->name << ", i64 " << lenComp << ")\n";

				(*m_ostr) << "call i8* @strncat(i8* %" << strptr->name << ", i8* %" << strCompptr->name << ", i64 3)\n";


				// separator ", " or "; "
				if(sym->ty == SymbolType::MATRIX && (i+1) % std::get<0>(sym->dims) == 0)
				{
					std::size_t idx0 = (i+1) % std::get<0>(sym->dims);
					std::size_t idx1 = i / std::get<0>(sym->dims);

					// don't output last "; "
					if(idx0 < std::get<1>(sym->dims)-1 && idx1 < std::get<1>(sym->dims)-1)
						(*m_ostr) << "call i8* @strncat(i8* %" << strptr->name << ", i8* %" << matsep->name << ", i64 3)\n";
				}
				else
				{
					// don't output last ", "
					if(i < num_floats-1)
						(*m_ostr) << "call i8* @strncat(i8* %" << strptr->name << ", i8* %" << vecsep->name << ", i64 3)\n";
				}
			}

			// vector end: " ]"
			(*m_ostr) << "call i8* @strncat(i8* %" << strptr->name << ", i8* %" << vecend->name << ", i64 3)\n";

			return str_mem;
		}
	}

	// unknown conversion
	else
	{
		throw std::runtime_error("Invalid type conversion.");
	}

	return nullptr;
}


/**
	* get the corresponding data type name
	*/
std::string LLAsm::get_type_name(SymbolType ty)
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


LLAsm::LLAsm(SymTab* syms) : m_syms{syms}
{}


t_astret LLAsm::visit(const ASTUMinus* ast)
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


t_astret LLAsm::visit(const ASTPlus* ast)
{
	t_astret term1 = ast->GetTerm1()->accept(this);
	t_astret term2 = ast->GetTerm2()->accept(this);

	// array types
	if(term1->ty == SymbolType::VECTOR || term1->ty == SymbolType::MATRIX)
	{
		if(term2->ty != term1->ty)
		{
			throw std::runtime_error("ASTPlus: Type mismatch in addition/subtraction of \""
				+ term1->name + "\" and \"" + term2->name + "\".");
		}

		if(std::get<0>(term1->dims) != std::get<0>(term2->dims))
		{
			throw std::runtime_error("ASTPlus: Dimension mismatch in addition/subtraction of \""
				+ term1->name + "\" and \"" + term2->name + "\".");
		}

		std::size_t dim = std::get<0>(term1->dims);
		if(term1->ty == SymbolType::MATRIX)
		{
			throw std::runtime_error("ASTPlus: Dimension mismatch in addition/subtraction of \""
				+ term1->name + "\" and \"" + term2->name + "\".");

			dim *= std::get<1>(term1->dims);
		}

		// allocate double array for result
		t_astret vec_mem = get_tmp_var(term1->ty, &term1->dims);
		(*m_ostr) << "%" << vec_mem->name << " = alloca [" << dim << " x double]\n";

		std::string op = ast->IsInverted() ? "fsub" : "fadd";

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
		t_astret elemptr_src1 = get_tmp_var();
		(*m_ostr) << "%" << elemptr_src1->name << " = getelementptr [" << dim << " x double], ["
			<< dim << " x double]* %" << term1->name << ", i64 0, i64 %" << ctrval->name << "\n";
		t_astret elemptr_src2 = get_tmp_var();
		(*m_ostr) << "%" << elemptr_src2->name << " = getelementptr [" << dim << " x double], ["
			<< dim << " x double]* %" << term2->name << ", i64 0, i64 %" << ctrval->name << "\n";

		t_astret elem_src1 = get_tmp_var();
		(*m_ostr) << "%" << elem_src1->name << " = load double, double* %" << elemptr_src1->name << "\n";
		t_astret elem_src2 = get_tmp_var();
		(*m_ostr) << "%" << elem_src2->name << " = load double, double* %" << elemptr_src2->name << "\n";

		t_astret elemptr_dst = get_tmp_var();
		(*m_ostr) << "%" << elemptr_dst->name << " = getelementptr [" << dim << " x double], ["
			<< dim << " x double]* %" << vec_mem->name << ", i64 0, i64 %" << ctrval->name << "\n";

		// add/subtract
		t_astret elem_dst = get_tmp_var(SymbolType::SCALAR);
		(*m_ostr) << "%" << elem_dst->name << " = " << op << " "
			<< "double %" << elem_src1->name << ", %" << elem_src2->name << "\n";

		// save result in array
		(*m_ostr) << "store double %" << elem_dst->name << ", double* %" << elemptr_dst->name << "\n";

		// increment counter
		t_astret newctrval = get_tmp_var(SymbolType::INT);
		(*m_ostr) << "%" << newctrval->name << " = add i64 %" << ctrval->name << ", 1\n";
		(*m_ostr) << "store i64 %" << newctrval->name << ", i64* %" << ctr->name << "\n";
		// ---------------

		(*m_ostr) << "br label %" << labelStart << "\n";
		(*m_ostr) << labelEnd << ":  ; loop end\n";

		return vec_mem;
	}

	// concatenate strings, TODO: conversion in case one term is not of string type
	else if(term1->ty == SymbolType::STRING || term2->ty == SymbolType::STRING)
	{
		/* get individual and total string lengths
		t_astret len1 = get_tmp_var(SymbolType::INT);
		t_astret len2 = get_tmp_var(SymbolType::INT);
		t_astret len = get_tmp_var(SymbolType::INT);
		t_astret len_arr = get_tmp_var(SymbolType::INT);

		(*m_ostr) << "%" << len1->name << " = " << "call i64 @strlen(i8* %" << strptr1->name << ")\n";
		(*m_ostr) << "%" << len2->name << " = " << "call i64 @strlen(i8* %" << strptr2->name << ")\n";
		(*m_ostr) << "%" << len->name << " = " << "add %" << len1->name << ", %" << len2->name << "\n";
		(*m_ostr) << "%" << len_arr->name << " = " << "add %" << len->name << ", 1\n";
		*/

		// get string pointers
		t_astret strptr1 = get_tmp_var();
		t_astret strptr2 = get_tmp_var();
		(*m_ostr) << "%" << strptr1->name << " = getelementptr [" << std::get<0>(term1->dims) << " x i8], ["
			<< std::get<0>(term1->dims) << " x i8]* %" << term1->name << ", i64 0, i64 0\n";
		(*m_ostr) << "%" << strptr2->name << " = getelementptr [" << std::get<0>(term2->dims) << " x i8], ["
			<< std::get<0>(term2->dims) << " x i8]* %" << term2->name << ", i64 0, i64 0\n";

		// allocate memory for concatenated string
		std::array<std::size_t, 2> dim{{ std::get<0>(term1->dims)+std::get<0>(term2->dims)-1, 0 }};
		t_astret res = get_tmp_var(SymbolType::STRING, &dim);

		// allocate the concatenated string's memory
		// TODO: use actual new string size, not the (maximum) allocated sizes of the terms in "dim"
		(*m_ostr) << "%" << res->name << " = alloca [" << std::get<0>(dim) << " x i8]\n";

		// get a pointer to the concatenated string
		t_astret resptr = get_tmp_var();
		(*m_ostr) << "%" << resptr->name << " = getelementptr [" << std::get<0>(dim) << " x i8], ["
			<< std::get<0>(dim) << " x i8]* %" << res->name << ", i64 0, i64 0\n";

		// copy first string
		(*m_ostr) << "call i8* @strncpy(i8* %" << resptr->name << ", i8* %" << strptr1->name
			<< ", i64 " << std::get<0>(dim) << ")\n";

		// concatenate second string
		(*m_ostr) << "call i8* @strncat(i8* %" << resptr->name << ", i8* %" << strptr2->name
			<< ", i64 " << std::get<0>(dim) << ")\n";

		return res;
	}

	// scalar types
	else
	{
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

	return nullptr;
}


t_astret LLAsm::visit(const ASTMult* ast)
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


t_astret LLAsm::visit(const ASTMod* ast)
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


t_astret LLAsm::visit(const ASTPow* ast)
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


t_astret LLAsm::visit(const ASTVar* ast)
{
	t_astret sym = get_sym(ast->GetIdent());
	if(sym == nullptr)
		throw std::runtime_error("ASTVar: Symbol \"" + ast->GetIdent() + "\" not in symbol table.");

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
		throw std::runtime_error("ASTVar: Invalid type for visited variable: \"" + sym->name + "\".");
	}

	return nullptr;
}


t_astret LLAsm::visit(const ASTCall* ast)
{
	const std::string& funcname = ast->GetIdent();
	t_astret func = get_sym(funcname);
	if(func == nullptr)
		throw std::runtime_error("ASTCall: Function \"" + funcname + "\" not in symbol table.");
	if(ast->GetArgumentList().size() != func->argty.size())
		throw std::runtime_error("ASTCall: Invalid number of function parameters for \"" + funcname + "\".");

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
		else if(arg_casted->ty == SymbolType::VECTOR || arg_casted->ty == SymbolType::MATRIX)
		{
			// array arguments are of type double*, so use a pointer to the array
			t_astret arrptr = get_tmp_var(arg_casted->ty, &arg_casted->dims);

			(*m_ostr) << "%" << arrptr->name << " = getelementptr ["
				<< std::get<0>(arg_casted->dims) << " x double], ["
				<< std::get<0>(arg_casted->dims) << " x double]* %"
				<< arg_casted->name << ", i64 0, i64 0\n";

			args.push_back(arrptr);
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


t_astret LLAsm::visit(const ASTStmts* ast)
{
	t_astret lastres = nullptr;

	for(const auto& stmt : ast->GetStatementList())
		lastres = stmt->accept(this);

	return lastres;
}


t_astret LLAsm::visit(const ASTVarDecl* ast)
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
			throw std::runtime_error("ASTVarDecl: Invalid type in declaration: \"" + sym->name + "\".");
		}
	}

	return nullptr;
}


t_astret LLAsm::visit(const ASTFunc* ast)
{
	m_curscope.push_back(ast->GetIdent());

	std::string rettype = get_type_name(ast->GetRetType());
	(*m_ostr) << "define " << rettype << " @" << ast->GetIdent() << "(";

	auto argnames = ast->GetArgNames();
	std::size_t idx=0;
	for(const auto& [argname, argtype, dim1, dim2] : argnames)
	{
		const std::string arg = std::string{"__arg_"} + argname;
		(*m_ostr) << get_type_name(argtype) << " %" << arg;
		if(idx < argnames.size()-1)
			(*m_ostr) << ", ";
		++idx;
	}
	(*m_ostr) << ")\n{\n";


	// create local copies of the arguments
	for(const auto& [argname, argtype, dim1, dim2] : argnames)
	{
		const std::string arg = std::string{"__arg_"} + argname;
		std::array<std::size_t, 2> argdims{{dim1, dim2}};

		t_astret symcpy = get_tmp_var(argtype, &argdims, &argname);

		if(argtype == SymbolType::SCALAR || argtype == SymbolType::INT)
		{
			std::string ty = get_type_name(argtype);
			(*m_ostr) << "%" << symcpy->name << " = alloca " << ty << "\n";
			(*m_ostr) << "store " << ty << " %" << arg << ", " << ty << "* %" << symcpy->name << "\n";
		}
		else if(argtype == SymbolType::STRING)
		{
			// allocate memory for local string copy
			(*m_ostr) << "%" << symcpy->name << " = alloca [" << std::get<0>(argdims) << " x i8]\n";

			t_astret strptr = get_tmp_var();
			(*m_ostr) << "%" << strptr->name << " = getelementptr [" << std::get<0>(argdims) << " x i8], ["
				<< std::get<0>(argdims) << " x i8]* %" << symcpy->name << ", i64 0, i64 0\n";

			// copy string
			(*m_ostr) << "call i8* @strncpy(i8* %" << strptr->name << ", i8* %" << arg
				<< ", i64 " << std::get<0>(argdims) << ")\n";
		}
		else if(argtype == SymbolType::VECTOR || argtype == SymbolType::MATRIX)
		{
			std::size_t argdim = std::get<0>(argdims);
			if(argtype == SymbolType::MATRIX)
				argdim *= std::get<1>(argdims);

			// allocate memory for local array copy
			(*m_ostr) << "%" << symcpy->name << " = alloca [" << argdim << " x double]\n";

			t_astret arrptr = get_tmp_var();
			(*m_ostr) << "%" << arrptr->name << " = getelementptr [" << argdim << " x double], ["
				<< argdim << " x double]* %" << symcpy->name << ", i64 0, i64 0\n";

			// copy array
			t_astret arrptr_cast = get_tmp_var();
			t_astret arg_cast = get_tmp_var();

			// cast to memcpy argument pointer type
			(*m_ostr) << "%" << arrptr_cast->name << " = bitcast double* %" << arrptr->name << " to i8*\n";
			(*m_ostr) << "%" << arg_cast->name << " = bitcast double* %" << arg << " to i8*\n";

			(*m_ostr) << "call i8* @memcpy(i8* %" << arrptr_cast->name << ", i8* %" << arg_cast->name
				<< ", i64 " << argdim*sizeof(double) << ")\n";
		}
		else
		{
			throw std::runtime_error("ASTFunc: Argument \"" + argname + "\" has invalid type.");
		}
	}


	t_astret lastres = ast->GetStatements()->accept(this);


	if(ast->GetRetType() == SymbolType::VOID)
	{
		(*m_ostr) << "ret void\n";
	}
	else
	{
		// TODO: string and array pointers cannot be returned as they refer to the local stack

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


t_astret LLAsm::visit(const ASTReturn* ast)
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


t_astret LLAsm::visit(const ASTAssign* ast)
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
		if(sym->ty == SymbolType::VECTOR && expr->ty == SymbolType::VECTOR)
		{
			if(std::get<0>(expr->dims) != std::get<0>(sym->dims))
				throw std::runtime_error("ASTAssign: Vector dimension mismatch in assignment of \"" + sym->name + "\".");
		}
		else if(sym->ty == SymbolType::MATRIX && expr->ty == SymbolType::MATRIX)
		{
			if(std::get<0>(expr->dims) != std::get<0>(sym->dims) && std::get<1>(expr->dims) != std::get<1>(sym->dims))
				throw std::runtime_error("ASTAssign: Matrix dimension mismatch in assignment of \"" + sym->name + "\".");
		}
		// TODO: check mat/vec and vec/mat cases


		std::size_t dim = std::get<0>(expr->dims);
		if(expr->ty == SymbolType::MATRIX)
		{
			if(std::get<1>(expr->dims) != std::get<1>(sym->dims))
				throw std::runtime_error("ASTAssign: Dimension mismatch in assignment of \"" + sym->name + "\".");

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
		//if(src_dim > dst_dim)	// TODO
		//	throw std::runtime_error("ASTAssign: Buffer of string \"" + sym->name + "\" is not large enough.");
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
		t_astret elemptr_src = get_tmp_var();
		(*m_ostr) << "%" << elemptr_src->name << " = getelementptr [" << src_dim << " x i8], ["
			<< src_dim << " x i8]* %" << expr->name << ", i64 0, i64 %" << ctrval->name << "\n";
		t_astret elemptr_dst = get_tmp_var();
		(*m_ostr) << "%" << elemptr_dst->name << " = getelementptr [" << dst_dim << " x i8], ["
			<< dst_dim << " x i8]* %" << sym->name << ", i64 0, i64 %" << ctrval->name << "\n";
		t_astret elem_src = get_tmp_var();
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


t_astret LLAsm::visit(const ASTComp* ast)
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


t_astret LLAsm::visit(const ASTCond* ast)
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


t_astret LLAsm::visit(const ASTLoop* ast)
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


t_astret LLAsm::visit(const ASTNumConst<double>* ast)
{
	double val = ast->GetVal();

	t_astret retvar = get_tmp_var(SymbolType::SCALAR);
	t_astret retvar2 = get_tmp_var(SymbolType::SCALAR);
	(*m_ostr) << "%" << retvar->name << " = alloca double\n";
	(*m_ostr) << "store double " << std::scientific << val << ", double* %" << retvar->name << "\n";
	(*m_ostr) << "%" << retvar2->name << " = load double, double* %" << retvar->name << "\n";

	return retvar2;
}


t_astret LLAsm::visit(const ASTNumConst<std::int64_t>* ast)
{
	std::int64_t val = ast->GetVal();

	t_astret retvar = get_tmp_var(SymbolType::INT);
	t_astret retvar2 = get_tmp_var(SymbolType::INT);
	(*m_ostr) << "%" << retvar->name << " = alloca i64\n";
	(*m_ostr) << "store i64 " << val << ", i64* %" << retvar->name << "\n";
	(*m_ostr) << "%" << retvar2->name << " = load i64, i64* %" << retvar->name << "\n";

	return retvar2;
}


t_astret LLAsm::visit(const ASTStrConst* ast)
{
	const std::string& str = ast->GetVal();
	std::size_t dim = str.length()+1;

	std::array<std::size_t, 2> dims{{dim, 0}};
	t_astret str_mem = get_tmp_var(SymbolType::STRING, &dims);

	// allocate the string's memory
	(*m_ostr) << "%" << str_mem->name << " = alloca [" << dim << " x i8]\n";

	// set the individual chars
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


t_astret LLAsm::visit(const ASTNumList<double>* ast)
{
	// array values and size
	const auto& lst = ast->GetList();
	std::size_t len = lst.size();
	std::array<std::size_t, 2> dims{{len, 0}};

	// allocate double array
	t_astret vec_mem = get_tmp_var(SymbolType::VECTOR, &dims);
	(*m_ostr) << "%" << vec_mem->name << " = alloca [" << len << " x double]\n";

	// set the individual array elements
	auto iter = lst.begin();
	for(std::size_t idx=0; idx<len; ++idx)
	{
		t_astret ptr = get_tmp_var();
		(*m_ostr) << "%" << ptr->name << " = getelementptr [" << len << " x double], ["
			<< len << " x double]* %" << vec_mem->name << ", i64 0, i64 " << idx << "\n";

		double val = *iter;
		(*m_ostr) << "store double " << val << ", double* %"  << ptr->name << "\n";
		++iter;
	}

	return vec_mem;
}
