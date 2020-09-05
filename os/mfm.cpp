/**
 * (m)fm coding test
 * @author Tobias Weber
 * @date 19-mar-19
 * @license: see 'LICENSE.EUPL' file
 *
 * @see https://en.wikipedia.org/wiki/Modified_Frequency_Modulation
 */

#include <iostream>
#include <string>
#include <sstream>


int main(int argc, char **argv)
{
	std::string strDat;
	std::cout << "Enter bits: ";
	std::cin >> strDat;

	if(strDat.length() == 0)
		return 0;

	std::ostringstream ostrBits;
	std::ostringstream ostrMFM;
	std::ostringstream ostrClock;

	char c0 = *strDat.begin();
	bool bPrev = (c0 != '0');
	strDat = strDat.substr(1);
	bool b = 0;

	for(char c : strDat)
	{
		b = (c != '0');

		ostrBits << bPrev << " ";
		ostrMFM << bPrev << !(bPrev || b);
		ostrClock << " " << !(bPrev || b);
		bPrev = b;
	}

	// last bit
	ostrBits << b;
	ostrMFM << b;

	std::cout << "Bits:  " << ostrBits.str() << std::endl;
	std::cout << "MFM:   " << ostrMFM.str() << std::endl;
	std::cout << "Clock: " << ostrClock.str() << std::endl;

	return 0;
}
