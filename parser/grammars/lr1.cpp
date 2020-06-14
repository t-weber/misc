/**
 * action and jump tables of LR(1) grammars
 * @author Tobias Weber
 * @date 10-dec-19
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- http://www.cs.ecu.edu/karl/5220/spr16/Notes/Bottom-up/slr1table.html
 *	- https://en.wikipedia.org/wiki/LR_parser
 *	- https://www.cs.uaf.edu/~cs331/notes/FirstFollow.pdf
 *	- http://www.cs.ecu.edu/karl/5220/spr16/Notes/Bottom-up/lr1.html
 *	- https://de.wikipedia.org/wiki/LL(k)-Grammatik
 *	- "Compilerbau Teil 1", ISBN: 3-486-25294-1, 1999, p. 267
 */

#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <tuple>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <functional>



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
	virtual ~Terminal() = default;

	virtual SymbolType GetType() const override { return SymbolType::TERM; }
};


std::shared_ptr<Terminal> g_eps, g_end;


/**
 * nonterminal symbols
 */
class NonTerminal : public Symbol
{
public:
	NonTerminal(const std::string& id) : Symbol{id} {}
	NonTerminal() = delete;
	virtual ~NonTerminal() = default;

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
	bool HasEpsRule(const std::shared_ptr<Symbol>& eps) const
	{
		for(const auto& rule : m_rules)
		{
			if(rule.size() == 1 && rule[0] == eps)
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
 * LR(1) grammar
 */
class LR1
{
protected:
	/**
	 * calculate first set
	 */
	static void CalcFirst(const std::shared_ptr<NonTerminal>& nonterm,
		std::map<std::string, std::set<std::shared_ptr<Symbol>>>& _first,
		std::map<std::string, std::vector<std::set<std::shared_ptr<Symbol>>>>* _first_perrule=nullptr)
	{
		// set already calculated?
		if(_first.find(nonterm->GetId()) != _first.end())
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

					// if the rule is left-recursive, ignore calculating the same symbol again
					if(symnonterm->GetId() != nonterm->GetId())
						CalcFirst(symnonterm, _first, _first_perrule);

					// add first set except eps
					bool bHasEps = false;
					for(const auto& symprod : _first[symnonterm->GetId()])
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

		_first[nonterm->GetId()] = first;

		if(_first_perrule)
			(*_first_perrule)[nonterm->GetId()] = first_perrule;
	}


	/**
	 * calculate follow set
	 */
	static void CalcFollow(const std::vector<std::shared_ptr<NonTerminal>>& nonterms,
		const std::shared_ptr<NonTerminal>& start,
		const std::shared_ptr<NonTerminal>& nonterm,
		std::map<std::string, std::set<std::shared_ptr<Symbol>>>& _first,
		std::map<std::string, std::set<std::shared_ptr<Symbol>>>& _follow)
	{
		// set already calculated?
		if(_follow.find(nonterm->GetId()) != _follow.end())
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
								for(const auto& symfirst : _first[rule[_iSym]->GetId()])
									if(!symfirst->IsEps())
										follow.insert(symfirst);

								const std::shared_ptr<NonTerminal>& symnonterm
									= reinterpret_cast<const std::shared_ptr<NonTerminal>&>(rule[_iSym]);

								if(!symnonterm->HasEpsRule(g_eps))
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

							if(!symnonterm->HasEpsRule(g_eps))
								break;
						}

						if(bLastSym || iNextSym==rule.size())
						{
							if(_nonterm != nonterm)
								CalcFollow(nonterms, start, _nonterm, _first, _follow);
							const auto& __follow = _follow[_nonterm->GetId()];
							follow.insert(__follow.begin(), __follow.end());
						}
					}
				}
			}
		}

		_follow[nonterm->GetId()] = follow;
	}


	/**
	 * TODO: for future extension towards LR(1) collections
	 */
	void CalcLRFollow(std::size_t cursor, const std::set<std::shared_ptr<Terminal>>& lhs_follows,
		const std::vector<std::shared_ptr<Symbol>>& rule)
	{
		// get rest of the rule after the cursor
		std::vector<std::shared_ptr<Symbol>> ruleaftercursor;
		for(std::size_t ruleidx=cursor+1; ruleidx<rule.size(); ++ruleidx)
			ruleaftercursor.push_back(rule[ruleidx]);

		for(const auto& lhs_follow : lhs_follows)
		{
			std::vector<std::shared_ptr<Symbol>> theruleaftercursor = ruleaftercursor;
			theruleaftercursor.push_back(lhs_follow);

			auto tmpNT = std::make_shared<NonTerminal>("tmp");
			tmpNT->AddRule(theruleaftercursor);

			std::map<std::string, std::set<std::shared_ptr<Symbol>>> tmp_first;
			CalcFirst(tmpNT, tmp_first);

			// has to have exactly 1 item
			if(tmp_first.size())
			{
				// add [NT after cursor, tmp_first] to collection
			}
		}
	}


	/**
	 * calculates SLR/LR(0) collection
	 */
	void CalcLRCollection(
		const std::vector<std::shared_ptr<NonTerminal>>& _lhs,
		const std::vector<std::vector<std::shared_ptr<Symbol>>> &rules,
		const std::vector<std::size_t>& cursors,
		std::size_t rulefrom = 0, const std::string *symTransition=nullptr)
	{
		using t_collection = std::vector<std::tuple<
			std::shared_ptr<NonTerminal>,		// lhs
			std::vector<std::shared_ptr<Symbol>>,	// rule
			std::size_t>				// cursor
		>;
		t_collection collection;


		// for memoisation of already calculated rules
		static std::set<std::string> memo_rules;
		std::set<std::string> cur_memo_rules;

		auto gethash = [](const auto& lhs, const auto& rulerhs, std::size_t cursor) -> std::string
		{
			// TODO: use a real hash function
			std::string hash;

			hash += lhs->GetId() + "#->#";
			for(const auto& sym : rulerhs)
				hash += sym->GetId() + "#,#";
			hash += "#;#";;
			hash += std::to_string(cursor);
			hash += "#|#";

			return hash;
		};

		std::function<void(const std::shared_ptr<NonTerminal>& _nonterm, std::set<std::string>& memo_rules)> addrhsrules;

		addrhsrules = [&collection, &addrhsrules, &gethash]
		(const std::shared_ptr<NonTerminal>& _nonterm, std::set<std::string>& memo_rules) -> void
		{
			for(std::size_t rulerhsidx=0; rulerhsidx<_nonterm->NumRules(); ++rulerhsidx)
			{
				auto& rulerhs = _nonterm->GetRule(rulerhsidx);
				std::string hash = gethash(_nonterm, rulerhs, 0);

				const auto memoIter = memo_rules.find(hash);
				if(memoIter == memo_rules.end())
				{
					memo_rules.insert(hash);

					collection.push_back(std::make_tuple(_nonterm, rulerhs, 0));

					// recursively add further nonterminals next to cursor
					if(rulerhs.size() && rulerhs[0]->GetType() == SymbolType::NONTERM)
						addrhsrules(reinterpret_cast<const std::shared_ptr<NonTerminal>&>(rulerhs[0]), memo_rules);
				}
			}
		};


		std::string hash;

		// iterate all relevant productions for given lhs
		for(std::size_t ruleidx=0; ruleidx<rules.size(); ++ruleidx)
		{
			const auto& lhs = _lhs[ruleidx];
			const auto& rule = rules[ruleidx];
			std::size_t cursor = cursors[ruleidx];

			//const auto& sym = rule[cursor];
			hash += gethash(lhs, rule, cursor);
		}


		// global numbering of rules
		static std::size_t rulectr = 0;
		static std::size_t memorulectr = 0;

		const auto memoIter = memo_rules.find(hash);
		if(memoIter == memo_rules.end())
		{
			memo_rules.insert(hash);

			// iterate all relevant productions for given lhs
			for(std::size_t ruleidx=0; ruleidx<rules.size(); ++ruleidx)
			{
				const auto& lhs = _lhs[ruleidx];
				const auto& rule = rules[ruleidx];
				std::size_t cursor = cursors[ruleidx];

				const auto& sym = rule[cursor];

				cur_memo_rules.insert(hash);
				collection.push_back(std::make_tuple(lhs, rule, cursor));

				// cursor at the end?
				if(cursor >= rule.size())
					continue;

				// non-terminal: need to insert productions
				if(sym->GetType() == SymbolType::NONTERM)
				{
					const auto& nonterm = reinterpret_cast<const std::shared_ptr<NonTerminal>&>(sym);
					addrhsrules(nonterm, cur_memo_rules);
				}
			}
		}
		else
		{
			// TODO, like output below
			std::size_t memorulenum = memorulectr++;
			std::cout << "memo item " << memorulenum << ": " << *memoIter << std::endl;
		}



		// --------------------------------------------------------------------
		// output collection
		std::size_t rulenum = rulectr++;
		std::cout << "item " << rulenum;
		if(symTransition)
		{
			std::cout << " (transition from item " << rulefrom << " with symbol " << *symTransition << ")";
			m_transitions[rulefrom].push_back(std::make_pair(rulenum, *symTransition));
		}

		if(collection.size())
			std::cout << ":";
		std::cout << std::endl;

		for(const auto& item : collection)
		{
			const auto& lhs = std::get<0>(item);
			const auto& rules = std::get<1>(item);
			const auto& cursor = std::get<2>(item);

			if(lhs)
				std::cout << "\t" << lhs->GetId() << " -> ";

			for(std::size_t iSym=0; iSym<rules.size(); ++iSym)
			{
				if(iSym == cursor)
					std::cout << ". ";
				std::cout << rules[iSym]->GetId() << " ";
			}
			// cursor at end?
			if(cursor >= rules.size())
				std::cout << ".";

			std::cout << "\n";
		}
		if(collection.size())
			std::cout << std::endl;
		// --------------------------------------------------------------------



		// --------------------------------------------------------------------
		// advance cursor
		// get possible transition symbols
		std::unordered_map<std::string, std::vector<std::size_t>> transSyms;
		for(std::size_t itemidx=0; itemidx<collection.size(); ++itemidx)
		{
			const auto& item = collection[itemidx];

			const auto& rule = std::get<1>(item);
			const auto& cursor = std::get<2>(item);

			if(cursor < rule.size())
				transSyms[rule[cursor]->GetId()].push_back(itemidx);
		}


		// for memoisation of already calculated collections
		static std::unordered_map<std::string, std::size_t> memo;

		// iterate possible transition symbols
		for(const auto& pair : transSyms)
		{
			std::vector<std::shared_ptr<NonTerminal>> nextlhs;
			std::vector<std::vector<std::shared_ptr<Symbol>>> nextrules;
			std::vector<std::size_t> nextcursors;
			std::string hash;

			const std::string& trans = pair.first;
			for(std::size_t itemidx : pair.second)
			{
				const auto& item = collection[itemidx];

				const auto& lhs = std::get<0>(item);
				const auto& rule = std::get<1>(item);
				std::size_t cursor = std::get<2>(item) + 1;

				nextlhs.push_back(lhs);
				nextrules.push_back(rule);
				nextcursors.push_back(cursor);

				hash += gethash(lhs, rule, cursor);
			}

			// not yet calculated?
			const auto memoIter = memo.find(hash);
			if(memoIter == memo.end())
			{
				memo.insert(std::make_pair(hash, rulenum));
				CalcLRCollection(nextlhs, nextrules, nextcursors, rulenum, &trans);
			}
			else
			{
				std::size_t memorulenum = memorulectr++;
				std::cout << "memo item " << memorulenum;
				std::cout << " (transition from item " << rulenum
					<< " with symbol " << trans << "): "
					<< "\n\tsame as the following transition from item " 
					<< memoIter->second << ":\n"
					<< memoIter->first << "\n"	// TODO: format output
					<< std::endl;

				const auto iter = m_transitions.find(memoIter->second);
				if(iter == m_transitions.end() || iter->second.size() == 0)
				{
					std::cerr << "Referenced invalid transition." << std::endl;
					exit(-1);
					//continue;
				}

				std::optional<std::size_t> rule_to;
				for(const auto& pair : iter->second)
				{
					// match production ident
					if(pair.second == trans)
					{
						rule_to = pair.first;
						break;
					}
				}

				if(!rule_to)
				{
					std::cerr << "Referenced invalid transition (2)." << std::endl;
					exit(-1);
					//continue;
				}

				// fill in transition rulenum -> memo_to into m_transitions
				m_transitions[rulenum].push_back(std::make_pair(*rule_to, trans));
			}
		}
		// --------------------------------------------------------------------
	}


	void WriteGraph(const std::string& file)
	{
		std::ofstream ofstr(file);
		if(!ofstr)
			return;

		ofstr << "digraph G_lr1\n{\n";


		// write states
		std::set<std::size_t> states;
		for(const auto& pair : m_transitions)
		{
			states.insert(pair.first);

			for(const auto& vecelem : pair.second)
				states.insert(vecelem.first);
		}

		for(std::size_t state : states)
			ofstr << "\t" << state << " [label=\"" << state << "\"];\n";


		// write transitions
		ofstr << "\n";
		for(const auto& pair : m_transitions)
		{
			std::size_t state_from = pair.first;
			for(const auto& [state_to, prod] : pair.second)
				ofstr << "\t" << state_from << " -> " << state_to << " [label=\"" << prod << "\"];\n";
		}


		ofstr << "}" << std::endl;
		ofstr.flush();
		ofstr.close();

		std::system("dot -Tsvg tmp.graph -o tmp.svg");
	}


public:
	LR1(const std::vector<std::shared_ptr<NonTerminal>>& nonterms,
		const std::shared_ptr<NonTerminal>& start)
		: m_nonterminals(nonterms), m_start(start)
	{
		// calculate first sets for all known non-terminals
		for(const auto& nonterm : nonterms)
			CalcFirst(nonterm, m_first, &m_first_perrule);

		// calculate follow sets for all known non-terminals
		for(const auto& nonterm : nonterms)
			CalcFollow(nonterms, start, nonterm, m_first, m_follow);

		// calculate the LR collection
		CalcLRCollection({{start}}, {{start->GetRule(0)}}, {0});

		WriteGraph("tmp.graph");
	}

	LR1() = delete;


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

	// for graph generation
	// transitions: [state_from, [state_to, production]]
	std::map<std::size_t, std::vector<std::pair<std::size_t, std::string>>> m_transitions;
};



std::ostream& operator<<(std::ostream& ostr, const LR1& lr1)
{
	ostr << "Productions:\n";
	for(const auto& nonterm : lr1.GetProductions())
	{
		ostr << "\t" << nonterm->GetId() << "\n\t\t-> ";
		for(std::size_t iRule=0; iRule<nonterm->NumRules(); ++iRule)
		{
			// rule
			const auto& rule = nonterm->GetRule(iRule);
			for(const auto& rhs : rule)
				ostr << rhs->GetId() << " ";

			// first set
			auto iter = lr1.GetFirstPerRule().find(nonterm->GetId());
			if(iter != lr1.GetFirstPerRule().end())
			{
				if(iRule < iter->second.size())
				{
					ostr << "\n\t\t\tFIRST: { ";
					const auto& first = iter->second[iRule];
					for(const auto& sym : first)
						ostr << sym->GetId() << ", ";
					ostr << " }";
				}
			}

			if(iRule < nonterm->NumRules()-1)
				ostr << "\n\t\t | ";
		}
		ostr << "\n";
	}


	ostr << "\nFIRST sets:\n";
	for(const auto& [id, set] : lr1.GetFirst() )
	{
		ostr << "\t" << std::left << std::setw(16) << id << ": { ";
		for(const auto& sym : set)
			ostr << sym->GetId() << ", ";
		ostr << " }\n";
	}

	ostr << "\nFOLLOW sets:\n";
	for(const auto& [id, set] : lr1.GetFollow() )
	{
		ostr << "\t" << std::left << std::setw(16) << id << ": { ";
		for(const auto& sym : set)
			ostr << sym->GetId() << ", ";
		ostr << " }\n";
	}

	return ostr;
}




// ----------------------------------------------------------------------------

int main()
{
	g_eps = std::make_shared<Terminal>("eps", true, false);
	g_end = std::make_shared<Terminal>("end", false, true);

	constexpr int example = 0;
	constexpr bool bSimplifiedGrammar = 0;

	if constexpr(example == 0)
	{
		// test grammar from: https://de.wikipedia.org/wiki/LL(k)-Grammatik#Beispiel
		auto start = std::make_shared<NonTerminal>("start");

		auto add_term = std::make_shared<NonTerminal>("add_term");
		auto mul_term = std::make_shared<NonTerminal>("mul_term");
		auto pow_term = std::make_shared<NonTerminal>("pow_term");
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

		start->AddRule({ add_term });

		add_term->AddRule({ add_term, plus, mul_term });
		if(!bSimplifiedGrammar)
			add_term->AddRule({ add_term, minus, mul_term });
		add_term->AddRule({ mul_term });

		if(bSimplifiedGrammar)
		{
			mul_term->AddRule({ mul_term, mult, factor });
		}
		else
		{
			mul_term->AddRule({ mul_term, mult, pow_term });
			mul_term->AddRule({ mul_term, div, pow_term });
			mul_term->AddRule({ mul_term, mod, pow_term });
		}

		if(bSimplifiedGrammar)
			mul_term->AddRule({ factor });
		else
			mul_term->AddRule({ pow_term });

		pow_term->AddRule({ pow_term, pow, factor });
		pow_term->AddRule({ factor });

		factor->AddRule({ bracket_open, add_term, bracket_close });
		if(!bSimplifiedGrammar)
		{
			factor->AddRule({ ident, bracket_open, bracket_close });			// function call
			factor->AddRule({ ident, bracket_open, add_term, bracket_close });			// function call
			factor->AddRule({ ident, bracket_open, add_term, comma, add_term, bracket_close });	// function call
		}
		factor->AddRule({ sym });


		LR1 lr1({start, add_term, mul_term, pow_term, factor}, start);
		std::cout << lr1 << std::endl;
	}

	else if constexpr(example == 1)
	{
		auto start = std::make_shared<NonTerminal>("start");

		auto A = std::make_shared<NonTerminal>("A");
		auto B = std::make_shared<NonTerminal>("B");

		auto a = std::make_shared<Terminal>("a");
		auto b = std::make_shared<Terminal>("b");

		start->AddRule({ A });

		A->AddRule({ a, b, B });
		A->AddRule({ b, b, B });
		B->AddRule({ b, B });

		LR1 lr1({start, A, B}, start);
		std::cout << lr1 << std::endl;
	}

	return 0;
}
