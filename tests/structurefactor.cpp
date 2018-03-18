/**
 * structure factor calculation test
 * @author Tobias Weber
 * @date 18-mar-18
 * @license: see 'LICENSE' file
 *
 * g++ -o structurefactor structurefactor.cpp -std=c++17 -fconcepts
 */

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <memory>

#include "../libs/math_algos.h"
#include "../libs/math_conts.h"
using namespace m;
using namespace m_ops;

using t_real = double;
using t_cplx = std::complex<t_real>;
using t_vec = std::vector<t_real>;
using t_mat = mat<t_real, std::vector>;
using t_vec_cplx = std::vector<t_cplx>;
using t_mat_cplx = mat<t_cplx, std::vector>;

std::string g_ws = " \t";


template<class T>
T from_str(const std::string& str)
{
	T t;

	std::istringstream istr(str);
	istr >> t;

	return t;
}


void calc(std::istream& istr)
{
	std::vector<t_vec_cplx> Ms;
	std::vector<t_cplx> bs;
	std::vector<t_vec> Rs;


	bool bNucl = 1;
	std::size_t linenr = 0;

	while(istr)
	{
		std::string line;
		std::getline(istr, line);
		++linenr;

		boost::trim_if(line, boost::is_any_of(g_ws));
		if(line == "")
			continue;

		std::vector<std::string> vectoks;
		boost::split(vectoks, line, boost::is_any_of(g_ws), boost::token_compress_on);

		t_real Rx = from_str<t_real>(vectoks[0]);
		t_real Ry = from_str<t_real>(vectoks[1]);
		t_real Rz = from_str<t_real>(vectoks[2]);

		Rs.emplace_back(create<t_vec>({Rx, Ry, Rz}));

		if(vectoks.size() == 4)		// nuclear
		{
			t_cplx b = from_str<t_cplx>(vectoks[3]);

			bs.emplace_back(b);
			bNucl = 1;
		}
		else if(vectoks.size() == 6)	// magnetic
		{
			t_real Mx = from_str<t_real>(vectoks[3]);
			t_real My = from_str<t_real>(vectoks[4]);
			t_real Mz = from_str<t_real>(vectoks[5]);

			Ms.emplace_back(create<t_vec_cplx>({Mx, My, Mz}));
			bNucl = 0;
		}
		else
		{
			std::cerr << "Error in line " << linenr << "." << std::endl;
			continue;
		}
	}


	std::cout << Rs.size() << " atom(s) defined.\n";

	for(t_real h=0; h<2; ++h)
		for(t_real k=0; k<2; ++k)
			for(t_real l=0; l<2; ++l)
			{
				auto Q = create<t_vec>({h,k,l});

				if(bNucl)
				{
					auto Fn = structure_factor<t_vec, t_cplx>(bs, Rs, Q);
					std::cout << "Fn(" << h << k << l << ") = "
						<< Fn << ", "
						<< "In(" << h << k << l << ") = "
						<< std::conj(Fn)*Fn << "\n";
				}
				else
				{
					auto Fm = structure_factor<t_vec, t_vec_cplx>(Ms, Rs, Q);
					std::cout << "Fm(" << h << k << l << ") = "
						<< Fm[0] << ", " <<  Fm[1] << ", " << "Fm[2]" << ")\n";
				}
			}
}


int main(int argc, char **argv)
{
	std::istream* pIstr = &std::cin;

	std::unique_ptr<std::ifstream> ifstr;
	if(argc > 1)
	{
		ifstr.reset(new std::ifstream(argv[1]));
		pIstr = ifstr.get();
	}

	calc(*pIstr);
	return 0;
}
