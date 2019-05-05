/**
 * set tests
 * @author Tobias Weber
 * @date may-19
 * @license: see 'LICENSE.EUPL' file
 */

#include <string>
#include <set>
#include <iostream>


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

	return 0;
}
