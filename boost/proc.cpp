/**
 * process tests
 * @author Tobias Weber
 * @date mar-2021
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_75_0/doc/html/boost_process/tutorial.html
 *  * https://www.boost.org/doc/libs/1_75_0/doc/html/process.html
 *  * https://www.boost.org/doc/libs/1_75_0/doc/html/boost/process/child.html
 *
 * g++ -std=c++20 -o proc proc.cpp -lpthread -lboost_filesystem
 */

#include <iostream>
#include <iterator>
#include <algorithm>

//#if __has_include(<filesystem>)
//	#pragma message("Using filesystem header.")
//	#include <filesystem>
//	namespace fs = std::filesystem;
//#else
//	#pragma message("Using Boost filesystem header.")
	#include <boost/filesystem.hpp>
	namespace fs = boost::filesystem;
//#endif

#include <boost/process.hpp>
namespace proc = boost::process;
namespace this_proc = boost::this_process;


int main(int argc, char** argv)
{
	// get process infos
	int pid = this_proc::get_id();
	proc::environment env{this_proc::environment()};
	std::vector<fs::path> paths{this_proc::path()};

	std::cout << "parent pid: " << pid << std::endl;

	std::cout << "Search paths:" << std::endl;
	for(const auto& path : paths)
		std::cout << "\t" << path << std::endl;

	/*std::cout << "Environment:" << std::endl;
	// TODO: get keys
	for(auto iter=env.begin(); iter!=env.end(); ++iter)
	{
		const auto& entry = *iter;
		std::vector<std::string> values = entry.to_vector();
		for(const auto& val : values)
			std::cout << val << ", ";
		std::cout << std::endl;
	}*/


	{
		proc::spawn("echo -e \"Test\"\n", env);
	}


	try
	{
		proc::ipstream outstream, errstream;
		proc::child ch{"nproc", proc::std_out>outstream, proc::std_err>errstream, env};

		ch.detach();
		std::cout << "child running: " << std::boolalpha << ch.running() << std::endl;
		//std::error_code err;
		//ch.terminate(err);
		//std::cout << "terminate error code: " << err.message() << std::endl;
		ch.wait();

		int i{};
		outstream >> i;
		std::cout << "nproc: " << i << "\n" << std::endl;
	}
	catch(const proc::process_error& err)
	{
		std::cerr << "Could not invoke process: " << err.what() << "." << std::endl;
	}


	try
	{
		proc::pipe pipe;
		proc::ipstream outstream, errstream, errstream2;

		// ls -a | sort -r
		fs::path path1 = proc::search_path("ls", paths);
		fs::path path2 = proc::search_path("sort", paths);

		if(!fs::exists(path1) || !fs::exists(path2))
		{
			std::cerr << "Tools not found." << std::endl;
			return -1;
		}

		std::cout << "Found tools: " << path1 << " and " << path2 << "." << std::endl;

		proc::group group;
		proc::child ch1{path1, "-a", proc::std_out>pipe, proc::std_err>errstream, env, group};
		proc::child ch2{path2, "-r", proc::std_in<pipe, proc::std_out>outstream, proc::std_err>errstream2, env, group};
		group.wait();

		std::copy(std::istream_iterator<std::string>{outstream}, std::istream_iterator<std::string>{},
			std::ostream_iterator<std::string>{std::cout, ", "});
		std::cout << "\n" << std::endl;
	}
	catch(const proc::process_error& err)
	{
		std::cerr << "Could not invoke process: " << err.what() << "." << std::endl;
	}

	return 0;
}
