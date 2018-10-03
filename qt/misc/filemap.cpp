/**
 * qt file mapping test
 * @author Tobias Weber
 * @date Oct-2018
 * @license: see 'LICENSE.GPL' file
 *
 * g++ -std=c++17 -fPIC -o filemap -I/usr/include/qt5 filemap.cpp -lQt5Core
 */

#include <QtCore/QFile>
#include <iostream>


int main(int argc, char** argv)
{
	if(QFile file("filemap.cpp"); file.open(QIODevice::ExistingOnly | QIODevice::ReadOnly))
	{
		unsigned char* mem = file.map(50 /*offset*/, 20 /*size*/);

		for(int i=0; i<10; ++i) std::cout << mem[i] << " ";
		std::cout << std::endl;

		file.unmap(mem);
		file.close();
	}
	else
	{
		std::cerr << "No such file." << std::endl;
		return -1;
	}

	return 0;
}

// ----------------------------------------------------------------------------
