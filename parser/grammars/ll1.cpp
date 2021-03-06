/**
 * first and follow sets of LL(1) grammars
 * @author Tobias Weber
 * @date 19-oct-19
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- https://www.cs.uaf.edu/~cs331/notes/FirstFollow.pdf
 *	- https://de.wikipedia.org/wiki/LL(k)-Grammatik
 */

#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <iomanip>



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
	Symbol(const std::string& id, bool bEps=false, bool bEnd=false)
		: m_id{id}, m_iseps{bEps}, m_isend{bEnd} {}
	Symbol() = delete;
	virtual ~Symbol() = default;

	virtual SymbolType GetType() const = 0;
	const std::string& GetId() const { return m_id; }

	bool IsEps() const { return m_iseps; }
	bool IsEnd() const { return m_isend; }


private:
	std::string m_id;
	bool m_iseps = false;
	bool m_isend = false;
};




/**
 * terminal symbols
 */
class Terminal : public Symbol
{
public:
	Terminal(const std::string& id, bool bEps=false, bool bEnd=false) : Symbol{id, bEps, bEnd} {}
	Terminal() = delete;

	virtual SymbolType GetType() const override { return SymbolType::TERM; }
};


std::shared_ptr<Terminal> g_eps, g_end;


/**
 * nonterminal symbols
 */
class NonTerminal : public Symbol
{
public:
	NonTerminal(const std::string& id) : Symbol{id}, m_rules{} {}
	NonTerminal() = delete;

	virtual SymbolType GetType() const override { return SymbolType::NONTERM; }


	/**
	 * add multiple alternative production rules
	 */
	void AddRule(const std::vector<std::shared_ptr<Symbol>>& rule)
	{
		m_rules.push_back(rule);
	}


	/**
	 * number of rules
	 */
	std::size_t NumRules() const { return m_rules.size(); }


	/**
	 * get a production rule
	 */
	const std::vector<std::shared_ptr<Symbol>>& GetRule(std::size_t i) const
	{
		return m_rules[i];
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
	 * does this non-terminal have a rule which produces epsilon?
	 */
	bool HasEpsRule() const
	{
		for(const auto& rule : m_rules)
		{
			if(rule.size() == 1 && rule[0]->IsEps())
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
 * LL(1) grammar
 */
class LL1
{
protected:
	/**
	 * calculate first set
	 */
	void CalcFirst(const std::shared_ptr<NonTerminal>& nonterm)
	{
		// set already calculated?
		if(m_first.find(nonterm->GetId()) != m_first.end())
			return;

		std::set<std::shared_ptr<Symbol>> first;
		std::vector<std::set<std::shared_ptr<Symbol>>> first_perrule;
		first_perrule.resize(nonterm->NumRules());

		// iterate rules
		for(std::size_t iRule=0; iRule<nonterm->NumRules(); ++iRule)
		{
			const auto& rule = nonterm->GetRule(iRule);

			// iterate RHS of rule
			for(std::size_t iSym=0; iSym<rule.size(); ++iSym)
			{
				const auto& sym = rule[iSym];

				// reached terminal symbol -> end
				if(sym->GetType() == SymbolType::TERM)
				{
					first.insert(sym);
					first_perrule[iRule].insert(sym);
					break;
				}
				// non-terminal
				else
				{
					const std::shared_ptr<NonTerminal>& symnonterm
						= reinterpret_cast<const std::shared_ptr<NonTerminal>&>(sym);
					CalcFirst(symnonterm);

					// add first set except eps
					bool bHasEps = false;
					for(const auto& symprod : m_first[symnonterm->GetId()])
					{
						if(symprod->IsEps())
						{
							bHasEps = true;

							// last non-terminal reached -> add epsilon
							if(iSym == rule.size()-1)
							{
								first.insert(symprod);
								first_perrule[iRule].insert(symprod);
							}

							continue;
						}

						first.insert(symprod);
						first_perrule[iRule].insert(symprod);
					}

					// no epsilon in production -> end
					if(!bHasEps)
						break;
				}
			}
		}

		m_first[nonterm->GetId()] = first;
		m_first_perrule[nonterm->GetId()] = first_perrule;
	}


	/**
	 * calculate follow set
	 */
	void CalcFollow(const std::vector<std::shared_ptr<NonTerminal>>& nonterms,
		const std::shared_ptr<NonTerminal>& start,
		const std::shared_ptr<NonTerminal>& nonterm)
	{
		// set already calculated?
		if(m_follow.find(nonterm->GetId()) != m_follow.end())
			return;

		std::set<std::shared_ptr<Symbol>> follow;


		// add end symbol as follower to start rule
		if(nonterm == start)
			follow.insert(g_end);


		// find current nonterminal in RHS of all rules (to get following symbols)
		for(const auto& _nonterm : nonterms)
		{
			// iterate rules
			for(std::size_t iRule=0; iRule<_nonterm->NumRules(); ++iRule)
			{
				const auto& rule = _nonterm->GetRule(iRule);

				// iterate RHS of rule
				for(std::size_t iSym=0; iSym<rule.size(); ++iSym)
				{
					// nonterm is in RHS of _nonterm rules
					if(rule[iSym]->GetId() == nonterm->GetId())
					{
						// add first set of following symbols except eps
						for(std::size_t _iSym=iSym+1; _iSym < rule.size(); ++_iSym)
						{
							// add terminal to follow set
							if(rule[_iSym]->GetType() == SymbolType::TERM
								&& !rule[_iSym]->IsEps())
							{
								follow.insert(rule[_iSym]);
								break;
							}
							else	// non-terminal
							{
								for(const auto& symfirst : m_first[rule[_iSym]->GetId()])
									if(!symfirst->IsEps())
										follow.insert(symfirst);

								const std::shared_ptr<NonTerminal>& symnonterm
									= reinterpret_cast<const std::shared_ptr<NonTerminal>&>(rule[_iSym]);

								if(!symnonterm->HasEpsRule())
									break;
							}
						}


						// last symbol in rule?
						bool bLastSym = (iSym+1 == rule.size());

						// ... or only epsilon productions afterwards?
						std::size_t iNextSym = iSym+1;
						for(; iNextSym<rule.size(); ++iNextSym)
						{
							if(rule[iNextSym]->GetType() == SymbolType::TERM)
								break;

							const std::shared_ptr<NonTerminal>& symnonterm
								= reinterpret_cast<const std::shared_ptr<NonTerminal>&>(rule[iNextSym]);

							if(!symnonterm->HasEpsRule())
								break;
						}

						if(bLastSym || iNextSym==rule.size())
						{
							if(_nonterm != nonterm)
								CalcFollow(nonterms, start, _nonterm);
							const auto& _follow = m_follow[_nonterm->GetId()];
							follow.insert(_follow.begin(), _follow.end());
						}
					}
				}
			}
		}


		m_follow[nonterm->GetId()] = follow;
	}


	bool HasLeftRecursions() const
	{
		for(const auto& nonterm : m_nonterminals)
		{
			for(std::size_t ruleidx=0; ruleidx<nonterm->NumRules(); ++ruleidx)
			{
				const auto& rule = nonterm->GetRule(ruleidx);
				if(rule.size() == 0)
					continue;

				if(rule[0]->GetId() == nonterm->GetId())
					return true;
			}
		}

		return false;
	}


public:
	LL1(const std::vector<std::shared_ptr<NonTerminal>>& nonterms,
		const std::shared_ptr<NonTerminal>& start)
	: m_nonterminals(nonterms), m_start(start), m_first{}, m_follow{}, m_first_perrule{}
	{
		if(HasLeftRecursions())
			throw std::runtime_error{"The given grammar has left recursions and is thus not of type LL(1)."};

		// calculate first sets for all known non-terminals
		for(const auto& nonterm : nonterms)
			CalcFirst(nonterm);

		// calculate follow sets for all known non-terminals
		for(const auto& nonterm : nonterms)
			CalcFollow(nonterms, start, nonterm);
	}

	LL1() = delete;


	const std::map<std::string, std::set<std::shared_ptr<Symbol>>>& GetFirst() const { return m_first; }
	const std::map<std::string, std::set<std::shared_ptr<Symbol>>>& GetFollow() const { return m_follow; }

	const std::map<std::string, std::vector<std::set<std::shared_ptr<Symbol>>>>& GetFirstPerRule() const { return m_first_perrule; }

	const std::vector<std::shared_ptr<NonTerminal>>& GetProductions() const { return m_nonterminals; }


private:
	// productions
	std::vector<std::shared_ptr<NonTerminal>> m_nonterminals;
	std::shared_ptr<NonTerminal> m_start;

	// first and follow sets
	std::map<std::string, std::set<std::shared_ptr<Symbol>>> m_first;
	std::map<std::string, std::set<std::shared_ptr<Symbol>>> m_follow;

	// per-rile first sets
	std::map<std::string, std::vector<std::set<std::shared_ptr<Symbol>>>> m_first_perrule;
};



std::ostream& operator<<(std::ostream& ostr, const LL1& ll1)
{
	ostr << "Productions:\n";
	for(const auto& nonterm : ll1.GetProductions())
	{
		ostr << "\t" << nonterm->GetId() << "\n\t\t-> ";
		for(std::size_t iRule=0; iRule<nonterm->NumRules(); ++iRule)
		{
			// rule
			const auto& rule = nonterm->GetRule(iRule);
			for(const auto& rhs : rule)
				ostr << rhs->GetId() << " ";

			// first set
			auto iterFirst = ll1.GetFirstPerRule().find(nonterm->GetId());
			if(iterFirst != ll1.GetFirstPerRule().end() && iRule < iterFirst->second.size())
			{
				ostr << "\n\t\t\tFIRST: { ";
				const auto& first = iterFirst->second[iRule];
				for(const auto& sym : first)
					ostr << sym->GetId() << ", ";
				ostr << " }";
			}

			if(iRule < nonterm->NumRules()-1)
				ostr << "\n\t\t | ";
		}
		ostr << "\n";
	}

	ostr << "\nFIRST sets:\n";
	for(const auto& [id, set] : ll1.GetFirst() )
	{
		ostr << "\t" << std::left << std::setw(16) << id << ": { ";
		for(const auto& sym : set)
			ostr << sym->GetId() << ", ";
		ostr << " }\n";
	}

	ostr << "\nFOLLOW sets:\n";
	for(const auto& [id, set] : ll1.GetFollow() )
	{
		ostr << "\t" << std::left << std::setw(16) << id << ": { ";
		for(const auto& sym : set)
			ostr << sym->GetId() << ", ";
		ostr << " }\n";
	}

	ostr << "\nLL(1) table:\n";
	for(const auto& nonterm : ll1.GetProductions())
	{
		auto iterFollow = ll1.GetFollow().find(nonterm->GetId());
		if(iterFollow == ll1.GetFollow().end())
			continue;

		for(std::size_t iRule=0; iRule<nonterm->NumRules(); ++iRule)
		{
			auto iterFirst = ll1.GetFirstPerRule().find(nonterm->GetId());
			if(iterFirst == ll1.GetFirstPerRule().end() || iRule >= iterFirst->second.size())
				continue;

			ostr << "\ttable[ " << nonterm->GetId() << ", ";

			bool eps_in_first = false;
			for(const auto& sym : iterFirst->second[iRule])
			{
				if(sym->IsEps())
					eps_in_first = true;
				else
					ostr << sym->GetId() << " ";
			}

			// also include follow if epsilon is in first
			if(eps_in_first)
			{
				for(const auto& sym : iterFollow->second)
					ostr << sym->GetId() << " ";
			}

			const auto& rule = nonterm->GetRule(iRule);
			ostr << "] = " << nonterm->GetId() << " -> ";
			for(const auto& sym : rule)
				ostr << sym->GetId() << " ";
			ostr << "\n";
		}
	}

	return ostr;
}




// ----------------------------------------------------------------------------


int main()
{
	g_eps = std::make_shared<Terminal>("eps", true, false);
	g_end = std::make_shared<Terminal>("end", false, true);

	try
	{
		// test grammar from: https://de.wikipedia.org/wiki/LL(k)-Grammatik#Beispiel
		// same grammar as in lr1.cpp, but with eliminated left-recursion
		auto add_term = std::make_shared<NonTerminal>("add_term");
		auto add_term_rest = std::make_shared<NonTerminal>("add_term_rest");
		auto mul_term = std::make_shared<NonTerminal>("mul_term");
		auto mul_term_rest = std::make_shared<NonTerminal>("mul_term_rest");
		auto pow_term = std::make_shared<NonTerminal>("pow_term");
		auto pow_term_rest = std::make_shared<NonTerminal>("pow_term_rest");
		auto factor = std::make_shared<NonTerminal>("factor");

		auto plus = std::make_shared<Terminal>("+");
		auto minus = std::make_shared<Terminal>("-");
		auto mult = std::make_shared<Terminal>("*");
		auto div = std::make_shared<Terminal>("/");
		auto mod = std::make_shared<Terminal>("%");
		auto pow = std::make_shared<Terminal>("^");
		auto bracket_open = std::make_shared<Terminal>("(");
		auto bracket_close = std::make_shared<Terminal>(")");
		auto comma = std::make_shared<Terminal>(",");
		auto sym = std::make_shared<Terminal>("symbol");
		auto ident = std::make_shared<Terminal>("ident");

		add_term->AddRule({ mul_term, add_term_rest });
		add_term->AddRule({ plus, mul_term, add_term_rest });		// unary +
		add_term->AddRule({ minus, mul_term, add_term_rest });		// unary -
		add_term_rest->AddRule({ plus, mul_term, add_term_rest });
		add_term_rest->AddRule({ minus, mul_term, add_term_rest });
		add_term_rest->AddRule({ g_eps });

		mul_term->AddRule({ pow_term, mul_term_rest });
		mul_term_rest->AddRule({ mult, pow_term, mul_term_rest });
		mul_term_rest->AddRule({ div, pow_term, mul_term_rest });
		mul_term_rest->AddRule({ mod, pow_term, mul_term_rest });
		mul_term_rest->AddRule({ g_eps });

		pow_term->AddRule({ factor, pow_term_rest });
		pow_term_rest->AddRule({ pow, factor, pow_term_rest });
		pow_term_rest->AddRule({ g_eps });

		factor->AddRule({ bracket_open, add_term, bracket_close });
		factor->AddRule({ ident, bracket_open, bracket_close });				// function call
		factor->AddRule({ ident, bracket_open, add_term, bracket_close });			// function call
		factor->AddRule({ ident, bracket_open, add_term, comma, add_term, bracket_close });	// function call
		factor->AddRule({ sym });

		LL1 ll1({add_term, add_term_rest, mul_term, mul_term_rest, pow_term, pow_term_rest, factor}, add_term);
		std::cout << ll1 << std::endl;
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;;
	}

	return 0;
}
