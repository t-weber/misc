/**
 * parser test - symbol table
 * @author Tobias Weber
 * @date 13-apr-20
 * @license: see 'LICENSE.GPL' file
 */

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <memory>
#include <string>
#include <unordered_map>
#include <array>
#include <iostream>


enum class SymbolType
{
	SCALAR,
	VECTOR,
	MATRIX,

	STRING,
	INT,
	VOID,

	FUNC,
};


struct Symbol
{
	std::string name;
	SymbolType ty = SymbolType::VOID;
	std::array<std::size_t, 2> dims{{0,0}};

	// for functions
	std::vector<SymbolType> argty{{}};
	SymbolType retty = SymbolType::VOID;
	std::array<std::size_t, 2> retdims{{0,0}};

	bool tmp = false;		// temporary or declared variable?
	bool on_heap = false;	// heap or stack variable?
};


class SymTab
{
public:
	const Symbol* AddSymbol(const std::string& name_with_scope,
		const std::string& name, SymbolType ty,
		const std::array<std::size_t, 2>& dims,
		bool is_temp=false, bool on_heap=false)
	{
		Symbol sym{.name = name, .ty = ty, .dims=dims, .tmp = is_temp, .on_heap=on_heap};
		auto pair = m_syms.insert_or_assign(name_with_scope, sym);
		return &pair.first->second;
	}


	const Symbol* AddFunc(const std::string& name_with_scope,
		const std::string& name, SymbolType retty,
		const std::vector<SymbolType>& argtypes,
		const std::array<std::size_t, 2>* retdims = nullptr)
	{
		Symbol sym{.name = name, .ty = SymbolType::FUNC, .argty = argtypes, .retty = retty};
		if(retdims)
			sym.retdims = *retdims;
		auto pair = m_syms.insert_or_assign(name_with_scope, sym);
		return &pair.first->second;
	}


	const Symbol* FindSymbol(const std::string& name) const
	{
		auto iter = m_syms.find(name);
		if(iter == m_syms.end())
			return nullptr;
		return &iter->second;
	}


	friend std::ostream& operator<<(std::ostream& ostr, const SymTab& tab)
	{
		for(const auto& pair : tab.m_syms)
			ostr << pair.first << " -> " << pair.second.name << "\n";

		return ostr;
	}


private:
	std::unordered_map<std::string, Symbol> m_syms;
};

#endif
