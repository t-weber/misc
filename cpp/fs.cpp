/**
 * path/file operations
 * @author Tobias Weber
 * @date 11-nov-17
 *
 * gcc -o fs fs.cpp -std=c++14 -lstdc++ -lstdc++fs
 * gcc -o fs fs.cpp -std=c++14 -lstdc++ -lboost_filesystem -lboost_system
 */

#include <iostream>

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#else
	#include <boost/filesystem.hpp>
	namespace fs = boost::filesystem;
#endif


void iterate_dir(const fs::path& path)
{
	using t_diriter = fs::directory_iterator;
	//using t_diriter = fs::recursive_directory_iterator;

	t_diriter diriter(path);
	for(t_diriter iter=fs::begin(diriter); iter!=fs::end(diriter); ++iter)
	{
		const fs::directory_entry& ent = *iter;
		const fs::path& entpath = ent;


		// ------------------------------------------------------------
		// example:

		//std::cout << "dir depth: " << iter.depth() << ", ";

		fs::path file = entpath.filename();
		fs::path file_noext = entpath.stem();
		fs::path ext = entpath.extension();
		fs::path parent = entpath.parent_path();

		if(fs::is_regular_file(entpath))
			std::cout << file << ": " << fs::file_size(entpath) << std::endl;
		else if(fs::is_directory(entpath))
			std::cout << entpath << std::endl;
		// ------------------------------------------------------------
	}
}


int main()
{
	std::cout << "Path char size: " << sizeof(fs::path::value_type) << std::endl;

	fs::path pathTmp = fs::temp_directory_path();
	if(fs::exists(pathTmp))
		std::cout << "tmp: " << pathTmp << std::endl;

	fs::path path = fs::current_path();
	std::cout << "cwd: " << path << std::endl;

	iterate_dir(path.parent_path());

	return 0;
}
