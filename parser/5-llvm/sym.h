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


enum class SymbolType
{
	SCALAR,
	VECTOR,
	MATRIX,

	STRING,
};


struct Symbol
{
	std::string name;
	SymbolType ty;
	std::array<unsigned int, 2> dims;
};


class SymTab
{
public:
	void AddSymbol(const std::string& name_with_scope,
		const std::string& name, SymbolType ty,
		const std::array<unsigned int, 2>& dims)
	{
		Symbol sym{.name = name, .ty = ty, .dims=dims};
		m_syms.insert(std::make_pair(name_with_scope, sym));
	}


private:
	std::unordered_map<std::string, Symbol> m_syms;
};

#endif
