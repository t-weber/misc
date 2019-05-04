/**
 * cyk algorithm, see: https://en.wikipedia.org/wiki/CYK_algorithm
 * @author Tobias Weber
 * @date 4-may-19
 * @license see 'LICENSE.EUPL' file
 */

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <boost/multi_array.hpp>



enum class SymbolType
{
	TERM,
	NONTERM
};



/**
 * symbol base class
 */
class Symbol
{
public:
	Symbol(const std::string& id) : m_id{id} {}
	Symbol() = delete;

	virtual SymbolType GetType() const = 0;

	const std::string& GetId() const { return m_id; }

private:
	std::string m_id;
};



/**
 * terminal symbols
 */
class Terminal : public Symbol
{
public:
	Terminal(const std::string& id) : Symbol{id} {}
	Terminal() = delete;

	virtual SymbolType GetType() const override { return SymbolType::TERM; }

private:
};



/**
 * nonterminal symbols
 */
class NonTerminal : public Symbol
{
public:
	NonTerminal(const std::string& id) : Symbol{id} {}
	NonTerminal() = delete;

	virtual SymbolType GetType() const override { return SymbolType::NONTERM; }


	/**
	 * add multiple alternative production rules
	 */
	void AddRule(const std::vector<std::shared_ptr<Symbol>>& m_rule)
	{
		m_rules.push_back(m_rule);
	}

	/**
	 * add a production rule
	 */
	void AddRule(const std::shared_ptr<Symbol>& m_rule)
	{
		m_rules.push_back({ m_rule });
	}


	/**
	 * does this non-terminal have a rule which produces the given rhs?
	 */
	bool HasRule(const std::vector<std::shared_ptr<Symbol>>& rhs) const
	{
		for(const auto& rule : m_rules)
		{
			if(rule.size() != rhs.size())
				continue;

			bool bMatch = 1;
			for(std::size_t i=0; i<rule.size(); ++i)
			{
				if(rule[i]->GetId() != rhs[i]->GetId())
				{
					bMatch = 0;
					break;
				}
			}

			if(bMatch)
				return true;
		}

		return false;
	}


	/**
	 * find all non-terminals in symbol list which have rules producing the given rhs
	 */
	static std::vector<std::shared_ptr<NonTerminal>> FindProducers(
		const std::vector<std::shared_ptr<NonTerminal>>& syms,
		const std::vector<std::shared_ptr<Symbol>>& rhs)
	{
		std::vector<std::shared_ptr<NonTerminal>> producers;

		for(const auto& sym : syms)
		{
			if(sym->HasRule(rhs))
				producers.push_back(sym);
		}

		return producers;
	}


	/**
	 * produce a production rule using all possible combinations of the given symbols
	 */
	static std::vector<std::vector<std::shared_ptr<Symbol>>>
	GenerateAllCombos(
		const std::vector<std::shared_ptr<NonTerminal>>& syms1,
		const std::vector<std::shared_ptr<NonTerminal>>& syms2)
	{
		std::vector<std::vector<std::shared_ptr<Symbol>>> rules;

		for(const auto& sym1 : syms1)
			for(const auto& sym2 : syms2)
				rules.emplace_back(std::vector<std::shared_ptr<Symbol>>{{ sym1, sym2 }});

		return rules;
	}


private:
	// production rules
	std::vector<std::vector<std::shared_ptr<Symbol>>> m_rules;
};




/**
 * CYK table
 */
class Cyk
{
public:
	Cyk(const std::vector<std::shared_ptr<NonTerminal>>& syms,
		const std::vector<std::shared_ptr<Terminal>>& input)
		: m_dim{input.size()}, m_tab{boost::extents[m_dim][m_dim]}
	{
		// main diagonal
		for(std::size_t i=0; i<m_dim; ++i)
			m_tab[i][i] = NonTerminal::FindProducers(syms, { input[i] });

		// sub-diagonals
		for(auto [i,j] : GenerateSubDiagIter())
		{
			// iterate all possible rhs rules
			std::size_t disttodiag = std::abs(std::ptrdiff_t(i)-std::ptrdiff_t(j));

			for(std::size_t k=0; k<disttodiag; ++k)
			{
				auto combos = NonTerminal::GenerateAllCombos(
					m_tab[i-k-1][j],
					m_tab[i][j+disttodiag-k]);

				for(const auto& combo : combos)
					InsertUniqueElems(m_tab[i][j], NonTerminal::FindProducers(syms, combo));
			}
		}
	}

	Cyk() = delete;

	std::size_t GetDim() const { return m_dim; }
	const std::vector<std::shared_ptr<NonTerminal>>& GetElem(std::size_t i, std::size_t j) const
	{
		return m_tab[i][j];
	}

protected:

	/**
	 * generate indices to iterate over the subdiagonal elements of the table
	 */
	std::vector<std::tuple<std::size_t, std::size_t>> GenerateSubDiagIter() const
	{
		std::vector<std::tuple<std::size_t, std::size_t>> indices;

		for(std::size_t sub=1; sub<m_dim; ++sub)
		{
			for(std::size_t i=0; i<m_dim; ++i)
			{
				if(i+sub >= m_dim)
					break;
				indices.emplace_back(std::make_tuple(i+sub, i));
			}
		}

		return indices;
	}

	/**
 	* insert symbol into a container if it doesn't already exist there
 	*/
	template<class t_cont, class t_sym>
	static bool InsertUniqueElem(t_cont& cont, const t_sym& sym)
	{
		for(const auto& existingsym : cont)
		{
			if(existingsym->GetId() == sym->GetId())
				return false;
		}

		cont.push_back(sym);
		return true;
	}

	/**
 	* insert symbols into a container if they don't already exist there
 	*/
	template<class t_cont, class t_vec>
	static void InsertUniqueElems(t_cont& cont, const t_vec& vec)
	{
		for(const auto& sym : vec)
			InsertUniqueElem(cont, sym);
	}

private:
	std::size_t m_dim = 0;
	boost::multi_array<std::vector<std::shared_ptr<NonTerminal>>, 2> m_tab;
};



std::ostream& operator<<(std::ostream& ostr, const Cyk& cyk)
{
	for(std::size_t i=0; i<cyk.GetDim(); ++i)
	{
		for(std::size_t j=0; j<cyk.GetDim(); ++j)
		{
			const auto& elems = cyk.GetElem(i,j);
			if(elems.size() == 0)
				ostr << "n/a";

			for(const auto& elem : elems)
				ostr << elem->GetId() << ", ";

			ostr << "; \t";
		}

		ostr << "\n";
	}

	return ostr;
}




// ----------------------------------------------------------------------------


int main()
{
	auto a = std::make_shared<Terminal>("a");
	auto b = std::make_shared<Terminal>("b");

	auto Start = std::make_shared<NonTerminal>("Start");
	auto A = std::make_shared<NonTerminal>("A");
	auto B = std::make_shared<NonTerminal>("B");

	Start->AddRule(A);
	Start->AddRule(B);
	Start->AddRule({ A, B });
	A->AddRule(a);
	B->AddRule(b);

	//std::cout << Start->HasRule({ A, B }) << std::endl;
	//std::cout << Start->HasRule({ A }) << std::endl;

	//auto producers = NonTerminal::FindProducers({Start, A, B}, {A, B});
	//for(const auto& producer : producers)
	//	std::cout << producer->GetId() << std::endl;

	Cyk cyk({Start, A, B}, { a, b });
	std::cout << cyk << std::endl;

	return 0;
}
