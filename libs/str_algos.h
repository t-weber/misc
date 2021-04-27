/**
 * string algos
 * @author Tobias Weber
 * @date apr-2021
 * @license see 'LICENSE.EUPL' file
 *
 * references:
 *   - (FUH 2021) "Effiziente Algorithmen" (2021), Kurs 1684, Fernuni Hagen
 *                (https://vu.fernuni-hagen.de/lvuweb/lvu/app/Kurs/01684).
 */

#ifndef __STR_ALGOS_H__
#define __STR_ALGOS_H__


#include <vector>
#include <unordered_map>
#include <queue>
#include <optional>
#include <memory>
#include <iostream>
#include <boost/dynamic_bitset.hpp>


/**
 * KMP pattern matching algo
 * @see (FUH 2021), Kurseinheit 3, p. 9 and p. 11.
 * @see https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm
 */
template<class t_str>
std::size_t find_pattern(const t_str& str, const t_str& pattern)
requires requires(const t_str& str, std::size_t idx)
{
	// needed t_str interface
	str[idx];
	str.size();
}
{
	std::size_t len_str = str.size();
	std::size_t len_pattern = pattern.size();


	auto get_prefix = [](const t_str& pattern, std::size_t len_pattern)
		-> std::vector<std::size_t>
	{
		std::vector<std::size_t> prefix{0};

		for(std::size_t pattern_pos=1; pattern_pos<len_pattern; ++pattern_pos)
		{
			std::size_t prefix_pos = prefix[pattern_pos-1];

			while(prefix_pos > 0 && pattern[prefix_pos] != pattern[pattern_pos])
				prefix_pos = prefix[prefix_pos-1];

			prefix.push_back(pattern[prefix_pos] == pattern[pattern_pos] ? prefix_pos+1 : 0);
		}

		prefix.insert(prefix.begin(), 0);
		return prefix;
	};


	auto prefix = get_prefix(pattern, len_pattern);
	//for(std::size_t i : prefix)
	//	std::cout << i << " ";
	//std::cout << std::endl;


	std::size_t str_pos = 0;
	std::size_t start_pos = 0;

	while(str_pos < len_str)
	{
		if(pattern[str_pos-start_pos] == str[str_pos])
		{
			if(str_pos-start_pos+1 == len_pattern)
				return start_pos;
			else
				++str_pos;
		}
		else
		{
			if(start_pos != str_pos)
			{
				start_pos = str_pos - prefix[str_pos-start_pos];
			}
			else
			{
				++start_pos;
				++str_pos;
			}
		}
	}

	// pattern not found
	return len_str;
}



template<class t_char>
struct HuffmanNode
{
	std::size_t freq{};
	std::optional<t_char> ch{std::nullopt};

	std::shared_ptr<HuffmanNode> left{}, right{};

	void print(std::ostream& ostr, std::size_t depth=0) const
	{
		for(std::size_t i=0; i<depth; ++i)
			std::cout << "\t";

		if(ch)
			ostr << "char = " << *ch << ", ";
		ostr << "freq = " << freq << "\n";

		if(left)
			left->print(ostr, depth+1);
		if(right)
			right->print(ostr, depth+1);
	}
};


/**
 * huffman code
 * @see (FUH 2021), Kurseinheit 2, p. 27
 * @see https://en.wikipedia.org/wiki/Huffman_coding
 */
template<class t_str>
std::shared_ptr<HuffmanNode<typename t_str::value_type>> huffman(const t_str& str)
{
	using t_char = typename t_str::value_type;
	using t_nodeptr = std::shared_ptr<HuffmanNode<t_char>>;


	// find frequency of characters
	std::unordered_map<t_char, std::size_t> freqs;
	for(const t_char& c : str)
	{
		auto iter = freqs.find(c);
		if(iter == freqs.end())
			std::tie(iter, std::ignore) = freqs.insert(std::make_pair(c, 0));

		++iter->second;
	}


	// insert characters and frequencies in priority queue
	auto queue_ordering =
		[](const t_nodeptr& node1, const t_nodeptr& node2) -> bool
		{
			return node1->freq > node2->freq;
		};

	std::priority_queue<t_nodeptr, std::vector<t_nodeptr>, decltype(queue_ordering)>
		queue(queue_ordering);

	for(const auto& pair : freqs)
	{
		t_nodeptr node = std::make_shared<HuffmanNode<t_char>>();
		node->ch = pair.first;
		node->freq = pair.second;

		queue.push(node);
	}


	// build huffman tree
	while(1)
	{
		if(queue.size() <= 1)
			break;

		t_nodeptr node1{std::move(queue.top())};
		queue.pop();
		t_nodeptr node2{std::move(queue.top())};
		queue.pop();

		t_nodeptr node = std::make_shared<HuffmanNode<t_char>>();
		node->left = node1;
		node->right = node2;
		node->freq = node1->freq + node2->freq;

		queue.push(node);
	}

	if(queue.size() == 0)
		return nullptr;
	return queue.top();
}


/**
 * get huffman bit encoding for characters from huffman tree
 * @see (FUH 2021), Kurseinheit 2, p. 27
 * @see https://en.wikipedia.org/wiki/Huffman_coding
 */
template<class t_str, class t_bits=boost::dynamic_bitset<unsigned long>>
std::unordered_map<typename t_str::value_type, t_bits>
huffman_mapping(const std::shared_ptr<HuffmanNode<typename t_str::value_type>>& tree)
{
	std::unordered_map<typename t_str::value_type, t_bits> map;

	std::function<void(decltype(tree), const t_bits&)> traverse;
	traverse = [&traverse, &map](
		const std::shared_ptr<HuffmanNode<typename t_str::value_type>>& node,
		const t_bits& pathbits) -> void
	{
		if(node->left)
		{
			auto leftpathbits = pathbits;
			leftpathbits.push_back(1);
			traverse(node->left, leftpathbits);
		}
		if(node->right)
		{
			auto rightpathbits = pathbits;
			rightpathbits.push_back(0);
			traverse(node->right, rightpathbits);
		}

		if(node->ch)
		{
			map.emplace(std::make_pair(*node->ch, pathbits));
		}
	};

	traverse(tree, t_bits{});

	return map;
}

#endif
