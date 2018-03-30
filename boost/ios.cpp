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
 * gcc -o ios ios.cpp -std=c++17 -lstdc++ -lboost_iostreams
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
		ios::mapped_file_source file("ios.cpp");
		std::cout << "mappled file size: " << file.size() << "\n";
		std::cout << file.data() << std::endl;
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

	return 0;
}
