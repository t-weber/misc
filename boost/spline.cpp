/**
 * spline interpolation test
 * @author Tobias Weber
 * @date 25-jan-2025
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *	https://live.boost.org/doc/libs/1_87_0/libs/math/doc/html/math_toolkit/cardinal_cubic_b.html
 *	https://live.boost.org/doc/libs/1_87_0/libs/math/doc/html/math_toolkit/cardinal_quadratic_b.html
 *	https://live.boost.org/doc/libs/1_87_0/libs/math/doc/html/math_toolkit/cardinal_quintic_b.html
 *	https://live.boost.org/doc/libs/1_87_0/libs/math/doc/html/math_toolkit/catmull_rom.html
 *
 * g++ -Wall -Wextra -std=c++20 -o spline spline.cpp
 * plot with:
 *	gnuplot -e "plot \"points.dat\" u 1:2 w points pt 7 ps 2, \"spline.dat\" u 1:2 w lines lw 2 title \"spline(x)\", \"spline.dat\" u 1:3 w lines lw 2 title \"spline'(x)\"" --persist
 *	gnuplot -e "plot \"points.dat\" u 1:2 w points pt 7 ps 2, \"spline4.dat\" u 2:3 w lines lw 2 title \"spline(x)\", \"spline4.dat\" u 2:5 w lines lw 2 title \"spline'(x)\"" --persist
 */

#include <vector>
#include <array>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include <boost/math/interpolators/cardinal_quadratic_b_spline.hpp>
#include <boost/math/interpolators/cardinal_quintic_b_spline.hpp>
#include <boost/math/interpolators/catmull_rom.hpp>
namespace math = boost::math;
namespace interp = math::interpolators;


using t_real = double;
//using t_pt = std::vector<t_real>;
using t_pt = std::array<t_real, 2>;
template<class...args> using t_cont = std::vector<args...>;


template<class t_spline, class t_real>
void write_spline(const t_spline& spline, t_real x0, t_real x1, t_real dx, const char* filename)
{
	std::ofstream ofstr_spline{filename};
	for(t_real x = x0; x <= x1; x += dx)
	{
		ofstr_spline
			<< std::setw(ofstr_spline.precision()*2) << std::left << x << " "
			<< std::setw(ofstr_spline.precision()*2) << std::left << spline(x) << " "
			<< std::setw(ofstr_spline.precision()*2) << std::left << spline.prime(x) << " "
			//<< std::setw(ofstr_spline.precision()*2) << std::left << spline.double_prime(x)
			<< std::endl;
	}

	std::cout << "Wrote " << filename << "." << std::endl;
}


template<class t_pt, class t_spline, class t_real>
void write_spline_pt(const t_spline& spline, t_real x0, t_real x1, t_real dx, const char* filename)
{
	std::ofstream ofstr_spline{filename};
	for(t_real x = x0; x <= x1; x += dx)
	{
		t_pt pos = spline(x);
		t_pt prime = spline.prime(x);

		ofstr_spline
			<< std::setw(ofstr_spline.precision()*2) << std::left << x << " "
			<< std::setw(ofstr_spline.precision()*2) << std::left << pos[0] << " "
			<< std::setw(ofstr_spline.precision()*2) << std::left << pos[1] << " "
			<< std::setw(ofstr_spline.precision()*2) << std::left << prime[0] << " "
			<< std::setw(ofstr_spline.precision()*2) << std::left << prime[1] << " "
			<< std::endl;
	}

	std::cout << "Wrote " << filename << "." << std::endl;
}


int main()
{
	// test data
	t_cont<t_real> datx{{ 1., 2., 3.,  4.,  5.,  6.,  7., 8. }};
	t_cont<t_real> daty{{ 5., 3., 1., 10., -4., -3.,  0., 1. }};

	// data as vectors
	t_cont<t_pt> vecs;
	vecs.reserve(datx.size());
	for(std::size_t i = 0; i < datx.size(); ++i)
		vecs.push_back({ datx[i], daty[i] });

	// splines
	interp::cardinal_cubic_b_spline spline{daty.begin(), daty.end(), *datx.begin(), datx[1] - datx[0]};
	interp::cardinal_quadratic_b_spline spline2{daty, *datx.begin(), datx[1] - datx[0]};
	interp::cardinal_quintic_b_spline spline3{daty, *datx.begin(), datx[1] - datx[0]};
	math::catmull_rom<t_pt> spline4(t_cont<t_pt>{vecs}, false, 0.5);

	// output data points
	std::ofstream ofstr_pts{"points.dat"};
	for(std::size_t i = 0; i < datx.size(); ++i)
	{
		ofstr_pts
			<< std::setw(ofstr_pts.precision()*2) << std::left << datx[i] << " "
			<< std::setw(ofstr_pts.precision()*2) << std::left << daty[i]
			<< std::endl;
	}
	std::cout << "Wrote points.dat." << std::endl;

	// output splines
	write_spline(spline, *datx.begin(), *datx.rbegin(), 0.1, "spline.dat");
	write_spline(spline2, *datx.begin(), *datx.rbegin(), 0.1, "spline2.dat");
	write_spline(spline3, *datx.begin(), *datx.rbegin(), 0.1, "spline3.dat");
	write_spline_pt<t_pt>(spline4, spline4.parameter_at_point(0),
		spline4.parameter_at_point(vecs.size() - 1), 0.1, "spline4.dat");

	return 0;
}
