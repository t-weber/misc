/**
 * converts an NFA to a DFA
 * @author Tobias Weber
 * @date aug-2025
 * @license see 'LICENSE.EUPL' file
 */

// g++ -std=c++23 -o nfa_to_dfa nfa_to_dfa.cpp

#include <vector>
#include <set>
#include <print>



template<class t_state = int, class t_symbol = int>
struct Transition
{
	t_state start{}, end{};
	t_symbol symbol;
};



template<class t_state = int, class t_symbol = int>
struct Automaton
{
	std::set<t_state> states{};
	std::set<t_symbol> symbols{};
	t_state start{};
	std::vector<t_state> end{};

	std::vector<Transition<t_state, t_symbol>> transitions{};
};



/**
 * get the end state of a transition from a given start state via a symbol
 */
template<class t_state = int, class t_symbol = int>
std::set<t_state> get_end_state(const Automaton<t_state, t_symbol>& A,
	const t_state& start, const t_symbol& sym)
{
	std::set<t_state> end;

	for(const Transition<t_state, t_symbol>& trans : A.transitions)
	{
		if(trans.start == start && trans.symbol == sym)
			end.insert(trans.end);
	}

	return end;
}



/**
 * converts an nfa to a dfa
 * @see https://de.wikipedia.org/wiki/Potenzmengenkonstruktion
 */
template<class t_state = int, class t_symbol = int>
std::pair<Automaton<t_state, t_symbol>, std::vector<std::set<t_state>>>
nfa_to_dfa(const Automaton<t_state, t_symbol>& nfa)
{
	Automaton<t_state, t_symbol> dfa;
	std::vector<std::set<t_state>> dfa_states;  // dfa states as collection of nfa states


	// state already known?
	auto has_state = [&dfa_states](const std::set<t_state>& state)
		-> std::pair<bool, t_state>
	{
		if(state.size() == 0)
			return std::make_pair(false, 0);

		t_state state_idx = 0;
		for(const std::set<t_state>& dfa_state : dfa_states)
		{
			//std::println("dfa_state = {}, state = {}.", dfa_state, state);
			if(dfa_state == state)
				return std::make_pair(true, state_idx);

			++state_idx;
		}

		return std::make_pair(false, 0);
	};


	// transition already known?
	auto has_transition = [&dfa](const Transition<t_state, t_symbol>& trans) -> bool
	{
		for(const Transition<t_state, t_symbol>& dfa_trans : dfa.transitions)
		{
			if(dfa_trans.start == trans.start &&
				dfa_trans.end == trans.end &&
				dfa_trans.symbol == trans.symbol)
			{
				return true;
			}
		}

		return false;
	};


	// start state and symbols
	dfa.start = 0;  // index into states
	dfa_states.emplace_back(std::set<t_state>({ nfa.start }));
	dfa.symbols = nfa.symbols;

	// continue as long as there are new states or transitions
	while(true)
	{
		bool added_state = false;
		bool added_transition = false;

		// iterate dfa states
		for(t_state dfa_state_idx = 0; dfa_state_idx < t_state(dfa_states.size()); ++dfa_state_idx)
		{
			//std::println("** Processing dfa state {} / {}-",
			//	dfa_state_idx, dfa_states.size());
			const auto/*&*/ nfa_states = dfa_states[dfa_state_idx];

			// iterate all symbols
			for(const t_symbol& sym : nfa.symbols)
			{
				std::set<t_state> reachable_nfa_states;

				// iterate nfa states for this dfa state
				for(const t_state& nfa_state : nfa_states)
				{
					// reachable states for this symbol
					reachable_nfa_states.merge(get_end_state(nfa, nfa_state, sym));
				}

				if(!reachable_nfa_states.size())
					continue;
				auto [state_found, prev_state_idx] = has_state(reachable_nfa_states);
				if(!state_found)
				{
					//std::println("New dfa state {} comprising nfa states {}.",
					//	dfa_states.size(), reachable_nfa_states);

					dfa_states.emplace_back(std::move(reachable_nfa_states));
					prev_state_idx = dfa_states.size() - 1;
					added_state = true;
				}

				// transitions
				Transition<t_state, t_symbol> trans
				{
					.start{dfa_state_idx},
					.end{prev_state_idx},
					.symbol{sym}
				};

				if(!has_transition(trans))
				{
					//std::println("New dfa transition {} -> {} via {}.",
					//	trans.start, trans.end, trans.symbol);

					dfa.transitions.emplace_back(std::move(trans));
					added_transition = true;
				}
			}
		}

		if(!added_state && !added_transition)
			break;
	}

	// add all dfa states
	for(t_state dfa_state_idx = 0; dfa_state_idx < int(dfa_states.size()); ++dfa_state_idx)
	{
		dfa.states.insert(dfa_state_idx);

		// add end states
		for(t_state nfa_end : nfa.end)
		{
			if(dfa_states[dfa_state_idx].find(nfa_end) != dfa_states[dfa_state_idx].end())
			{
				dfa.end.push_back(dfa_state_idx);
				continue;
			}
		}
	}

	return std::make_pair(dfa, dfa_states);
}



template<class t_state = int, class t_symbol = int>
void print(const Automaton<t_state, t_symbol>& A)
{
	std::print("States: ");
	for(const t_state& state : A.states)
		std::print("{} ", state);
	std::println();

	std::print("Symbols: ");
	for(const t_symbol& sym : A.symbols)
		std::print("{} ", sym);
	std::println();

	std::println("Start state: {}", A.start);

	std::print("End states: ");
	for(const t_state& state : A.end)
		std::print("{} ", state);
	std::println();

	std::println("Transitions:");
	std::print("\t{:<10}", "state");
	for(const t_symbol& sym : A.symbols)
		std::print("{:<10}", sym);
	std::println();
	for(const t_state& state : A.states)
	{
		std::print("\t{:<10}", state);

		for(const t_symbol& sym : A.symbols)
		{
			std::set<t_state> ends = get_end_state(A, state, sym);
			if(ends.size())
				std::print("{:10}", ends);
			else
				std::print("{:10}", "--");
		}

		std::println();
	}
}


int main()
{
	using t_state = int;
	using t_sym = char;

	Automaton<t_state, t_sym> nfa
	{
		.states{0, 1, 2},
		.symbols{'a', 'b'},
		.start = 0, .end = std::vector<t_state>({3})
	};

	nfa.transitions.emplace_back(
		Transition<t_state, t_sym>{.start = 0, .end = 0, .symbol = 'a'});
	nfa.transitions.emplace_back(
		Transition<t_state, t_sym>{.start = 0, .end = 0, .symbol = 'b'});
	nfa.transitions.emplace_back(
		Transition<t_state, t_sym>{.start = 0, .end = 1, .symbol = 'b'});
	nfa.transitions.emplace_back(
		Transition<t_state, t_sym>{.start = 1, .end = 2, .symbol = 'a'});

	std::println("NFA:");
	print(nfa);
	std::println();

	auto [dfa, dfa_states] = nfa_to_dfa(nfa);

	std::println("DFA:");
	print(dfa);

	std::println("State correspondance:");
	std::println("\t{:<15}{:<15}", "DFA state", "NFA states");
	t_state dfa_state_idx = 0;
	for(const std::set<t_state>& nfa_states : dfa_states)
	{
		std::println("\t{:<15}{:<15}", dfa_state_idx, nfa_states);
		++dfa_state_idx;
	}

	std::println();
	return 0;
}
