/**
 * property tree code snippets
 * @author Tobias Weber
 * @date 18-nov-17
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_65_1/doc/html/property_tree/reference.html
 *  * https://github.com/boostorg/property_tree/blob/develop/examples/debug_settings.cpp
 *  * https://github.com/boostorg/property_tree/blob/develop/test/test_property_tree.hpp
 *
 * gcc -o ptree ptree.cpp -std=c++17 -lstdc++ -lboost_iostreams
 */

#include <iostream>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
namespace ptree = boost::property_tree;

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
namespace ios = boost::iostreams;


// ----------------------------------------------------------------------------
// type name helper
#include <boost/type_index.hpp>
namespace ty = boost::typeindex;

std::string get_type_str(const auto& obj)
{
	return ty::type_id_with_cvr<decltype(obj)>().pretty_name();
}
// ----------------------------------------------------------------------------


int main()
{
	// write
	{
		ptree::ptree prop;

		// put values
		prop.put<int>("root.test", 123);
		prop.put<double>("root.test2", 456.789);
		prop.put<std::string>("root.test3", "123456");

		// write to files
		ptree::write_json("test.json", prop);
		ptree::write_info("test.info", prop, std::locale(), ptree::info_writer_make_settings('\t', 1));

		// only for xml: attributes
		prop.put<int>("root.<xmlattr>.test", 123);

		// write to an ostream
		std::ostringstream ostr;
		ptree::write_xml(ostr, prop, ptree::xml_writer_make_settings('\t', 1, std::string("utf-8")));
		std::cout << ostr.str() << std::endl;
	}


	// read
	{
		ptree::ptree prop;
		try
		{
			ptree::read_json("test.json", prop);
		}
		catch(const std::exception& ex)
		{
			std::cerr << ex.what() << std::endl;
			return -1;
		}

		// get value
		std::cout << prop.get<int>("root.test", 0) << std::endl;
		std::cout << *prop.get_optional<int>("root.test") << std::endl;

		// iterate over subtree (returns an optional<ptree>)
		const auto& children = prop.get_child_optional("root");
		//for(const auto& child : prop)
		for(const auto& child : *children)
		{
			//std::cout << get_type_str(child.second) << std::endl;
			std::cout << child.first << ": ";
			std::cout << *child.second.get_optional<std::string>("") << std::endl;
		}


		// find unique matching node
		if(auto iter = prop.find("root"); iter != prop.not_found())
		{
			//std::cout << get_type_str(iter) << std::endl;
			std::cout << iter->first << std::endl;
		}
		else
		{
			std::cerr << "Node not found." << std::endl;
		}

	}

	// find equal ranges
	{
		ptree::ptree prop;
		ptree::ptree prop2;
		prop2.put<int>("a", 123);
		prop2.put<int>("b", 456);

		// twice the same key
		prop.add_child("test", prop2);
		prop.add_child("test", prop2);

		ptree::write_xml("test2.xml", prop,
			std::locale(), ptree::xml_writer_make_settings('\t', 1, std::string("utf-8")));


		// find all matching nodes
		ptree::ptree tree;
		if(auto [begin, end] = prop./*get_child("root").*/equal_range("test"); begin!=end)
		{
			std::cout << begin->first << std::endl;
		}
		else
		{
			std::cerr << "Range not found." << std::endl;
		}
	}


	// xml attributes
	{
		std::stringstream istr("<test a=\"test123\" b=\"456\" />");

		ptree::ptree prop;
		ptree::read_xml(istr, prop);

		auto attrs = prop.get_child("test.<xmlattr>");
		std::cout << attrs.size() << " attributes" << std::endl;
		std::cout << "a = " << prop.get<std::string>("test.<xmlattr>.a") << std::endl;
	}


	// write to a filtering ostream
	{
		ptree::ptree prop;

		// put values
		prop.put<int>("root.test", 123);
		prop.put<double>("root.test2", 456.789);
		prop.put<std::string>("root.test3", "123456");

		ios::stream<ios::file_sink> file("tst.xml.bz2");
		ios::filtering_ostream fostr;
		fostr.push(ios::bzip2_compressor());
		fostr.push(file);
		ptree::write_xml(fostr, prop, ptree::xml_writer_make_settings('\t', 1, std::string("utf-8")));
	}


	// read from a filtering istream
	{
		ios::stream<ios::file_source> file("tst.xml.bz2");

		ios::filtering_istream istr;
		istr.push(ios::bzip2_decompressor());
		istr.push(file);

		ptree::ptree prop;
		ptree::read_xml(istr, prop);

		std::cout << prop.get<double>("root.test2") << std::endl;
	}

	return 0;
}
