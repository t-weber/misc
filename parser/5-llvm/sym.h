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
#include <iostream>


enum class SymbolType
{
	SCALAR,
	STRING,
	INT,
	FUNC,
	VOID,
};


struct Symbol
{
	std::string name;
	SymbolType ty;

	// for functions
	std::vector<SymbolType> argty;
	SymbolType retty;

	bool tmp = false;	// temporary variable?
};


class SymTab
{
public:
	const Symbol* AddSymbol(const std::string& name_with_scope,
		const std::string& name, SymbolType ty,
		bool is_temp=false)
	{
		Symbol sym{.name = name, .ty = ty, .tmp = is_temp};
		auto pair = m_syms.insert_or_assign(name_with_scope, sym);
		return &pair.first->second;
	}


	const Symbol* AddFunc(const std::string& name_with_scope,
		const std::string& name, SymbolType retty,
		const std::vector<SymbolType>& argtypes)
	{
		Symbol sym{.name = name, .ty = SymbolType::FUNC, .argty = argtypes, .retty = retty};
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
