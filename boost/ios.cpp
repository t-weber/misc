/**
 * iostreams tests and snippets
 * @author Tobias Weber
 * @date 19-nov-17
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/libs/iostreams/doc/index.html
 *  * https://github.com/boostorg/iostreams/tree/develop/example
 *
 * g++ -o ios ios.cpp -std=c++17 -lboost_iostreams
 */

#include <iostream>
#include <string>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
namespace ios = boost::iostreams;


class TestOutFilter
{
	public:
		using char_type = char;
		struct category : public ios::output_filter_tag {};

		template<class Snk>
		bool put(Snk& snk, char_type c)
		{
			// replace '1' by "one"
			if(c == '1')
				return ios::put(snk, 'o') && ios::put(snk, 'n') && ios::put(snk, 'e');

			// else output unchanged
			return ios::put(snk, c);
		}
};


class TestInFilter
{
public:
	using char_type = char;
	struct category : public ios::input_filter_tag {};

	template<class Src>
	char_type get(Src& src)
	{
		while(true)
		{
			char_type c = ios::get(src);
			if(c < 0)	// EOF?
				return c;

			// filter out all letters
			if(!std::isalpha(c))
				return c;
		}

		return 0;
	}
};


int main()
{
	// load file
	{
		ios::stream<ios::file_source> ios("ios.cpp");
		std::string str;
		std::getline(ios, str);
		std::cout << str << "\n";
	}


	// container sink
	{
		std::vector<char> vec;
		ios::filtering_ostream ios(ios::back_inserter(vec));
		ios << 123 << 456 << std::flush;
		std::copy(vec.begin(), vec.end(), std::ostream_iterator<char>(std::cout, ", "));
		std::cout << "\n";
	}


	// filter chain
	{
		ios::stream<ios::file_sink> file("tst.txt.bz2");
		//ios::filtering_ostream ios(std::cout);
		//ios::filtering_ostream ios(std::ostream_iterator<char>(std::cout, ", "));
		ios::filtering_ostream ios;
		ios.push(ios::bzip2_compressor());
		ios.push(file);
		ios << "Test\n1234\n";
	}


	// filter chain 2
	{
		ios::stream<ios::file_source> file("tst.txt.bz2");
		//ios::stream<ios::mapped_file_source> file("tst.txt.bz2");

		ios::filtering_istream ios;
		ios.push(ios::bzip2_decompressor());
		ios.push(file);

		std::copy(std::istream_iterator<char>(ios), std::istream_iterator<char>(),
			std::ostream_iterator<char>(std::cout, ""));
		std::cout << "\n";
	}


	// mapped file
	{
		int align = ios::mapped_file_source::alignment();
		std::cout << "Mem alignment size: " << align << std::endl;
		ios::mapped_file_source file("ios.cpp", 16 /*len*/, 0*align /*offs*/);
		std::cout << "mapped file size: " << file.size() << "\n";
		//std::cout << file.data() << std::endl;
		for(char c : file) std::cout << c;
		std::cout << std::endl;
	}


	// mapped file 2
	{
		// the file has to exist with size!=0
		//ios::mapped_file_sink file("tst.txt");

		// create a non-empty file first
		ios::mapped_file_params params("tst.txt");
		params.new_file_size = 1;
		ios::mapped_file_sink file(params);

		file.resize(1024);
		for(std::size_t i=0; i<1024; ++i)
			file.data()[i] = '0';
	}


	// custom output filter
	{
		ios::stream<ios::file_sink> file("test.txt");
		ios::filtering_ostream ios;
		ios.push(TestOutFilter());
		ios.push(file);
		ios << "Test\n1234\n";
	}


	// custom input filter
	{
		ios::stream<ios::file_source> file("test.txt");

		ios::filtering_istream ios;
		ios.push(TestInFilter());
		ios.push(file);

		std::copy(std::istream_iterator<char>(ios), std::istream_iterator<char>(),
			std::ostream_iterator<char>(std::cout, ""));
		std::cout << std::endl;
	}


	return 0;
}
