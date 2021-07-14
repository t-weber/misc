/**
 * set tests
 * @author Tobias Weber
 * @date may-19
 * @license: see 'LICENSE.EUPL' file
 *
 * references:
 *	- https://www.boost.org/doc/libs/1_76_0/doc/html/hash/combine.html
 */

#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <iostream>

#include <boost/functional/hash.hpp>


template<class T>
std::size_t unordered_hash(const T& t)
{
	return std::hash<T>{}(t);
}


template<class T1, class ...T2>
std::size_t unordered_hash(const T1& t1, const T2& ...t2)
{
	std::size_t hash1 = unordered_hash<T1>(t1);
	std::size_t hash2 = unordered_hash<T2...>(t2...);

	std::size_t hash_comb1 = 0;
	boost::hash_combine(hash_comb1, hash1);
	boost::hash_combine(hash_comb1, hash2);

	std::size_t hash_comb2 = 0;
	boost::hash_combine(hash_comb2, hash2);
	boost::hash_combine(hash_comb2, hash1);

	// order matters for boost::hash_combine -> sort hashes to make it unordered
	if(hash_comb2 < hash_comb1)
		std::swap(hash_comb1, hash_comb2);

	std::size_t hash_comb = 0;
	boost::hash_combine(hash_comb, hash_comb1);
	boost::hash_combine(hash_comb, hash_comb2);

	//std::cout << "hash1 = " << hash1 << ", hash2 = " << hash2 << ", hash_comb = " << hash_comb << std::endl;
	return hash_comb;
}


int main()
{
	{
		auto lenCompare = [](const std::string& str1, const std::string& str2) -> bool
		{
			std::cout << "Comparing " << str1 << " and " << str2 << "." << std::endl;
			return str1.length() < str2.length();
		};

		std::set<std::string, decltype(lenCompare)> set{{}, lenCompare};
		set.insert("1");
		set.insert("12");
		set.insert("123");
		set.insert("a");
		set.insert("ab");
		set.insert("abc");

		for(const auto& s : set)
			std::cout << s << ", ";
		std::cout << std::endl;
	}


	std::cout << std::endl;


	{
		// test unordered hash function
		std::cout << unordered_hash(1, 2) << std::endl;
		std::cout << unordered_hash(2, 1) << std::endl;


		using t_key = std::pair<int, int>;

		struct t_hash
		{
			std::size_t operator()(const t_key& key) const
			{
				std::cout << __PRETTY_FUNCTION__ << std::endl;
				return unordered_hash(std::get<0>(key), std::get<1>(key));
			}
		};

		struct t_equ
		{
			bool operator()(const t_key& a, const t_key& b) const
			{
				std::cout << __PRETTY_FUNCTION__ << std::endl;

				bool ok1 = (std::get<0>(a) == std::get<0>(b)) && (std::get<1>(a) == std::get<1>(b));
				bool ok2 = (std::get<0>(a) == std::get<1>(b)) && (std::get<1>(a) == std::get<0>(b));
				return ok1 || ok2;
			}
		};

		std::unordered_set<t_key, t_hash, t_equ> set{10};
		set.emplace(std::make_pair(1,2));
		set.emplace(std::make_pair(2,1));
		set.emplace(std::make_pair(3,4));
		set.emplace(std::make_pair(3,5));
		set.emplace(std::make_pair(3,6));
		set.emplace(std::make_pair(3,7));
		set.emplace(std::make_pair(5,3));

		std::cout << std::endl;
		for(const t_key& key : set)
			std::cout << "key: " << std::get<0>(key) << ", " << std::get<1>(key) << std::endl;


		using t_val = int;

		std::unordered_map<t_key, t_val, t_hash, t_equ> map{10};
		map.emplace(std::make_pair(std::make_pair(1,2), 999));
		map.emplace(std::make_pair(std::make_pair(2,3), 500));
		map.emplace(std::make_pair(std::make_pair(3,4), -123));

		std::cout << std::endl;
		for(const auto& pair : map)
		{
			const t_key& key = std::get<0>(pair);
			const t_val val = std::get<1>(pair);

			std::cout << "key: " << std::get<0>(key) << ", " << std::get<1>(key);
			std::cout << "; value: " << val << std::endl;
		}

		std::cout << map[std::make_pair(1, 2)] << std::endl;
		std::cout << map[std::make_pair(2, 1)] << std::endl;
	}

	return 0;
}
