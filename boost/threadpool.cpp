/**
 * thead pool and threading overhead test
 * @author Tobias Weber
 * @date 15-dec-18
 * @license: see 'LICENSE.EUPL' file
 *
 * Reference:
 *  * https://www.boost.org/doc/libs/1_69_0/doc/html/boost_asio/reference/thread_pool.html
 *
 * g++ -std=c++17 -O2 -march=native -o threadpool threadpool.cpp -lboost_system -lpthread
 */

#include <random>
#include <atomic>
#include <chrono>
#include <complex>
#include <iostream>
#include <fstream>
#include <boost/asio.hpp>


using t_real = double;
using t_clock = std::chrono::steady_clock;
//using t_clock = std::chrono::high_resolution_clock;

const std::size_t g_seed = std::random_device{}();


bool is_in_circle(t_real rad, t_real x, t_real y)
{
	t_real r = std::sqrt(x*x + y*y);
	return r <= rad;
}


std::tuple<bool, t_real> is_in_mandel(t_real lim, std::size_t iter, t_real x, t_real y)
{
	std::complex<t_real> z(0., 0.);
	std::complex<t_real> pos(x, y);

	for(std::size_t i=0; i<iter; ++i)
		z = z*z + pos;

	t_real norm = std::norm(z);
	bool isinside = norm <= lim*lim;

	return std::make_tuple(isinside, norm);
}


t_real calc_pi(std::size_t N, unsigned int Nthreads=4)
{
	std::mt19937_64 gen;
	gen.seed(g_seed);

	std::atomic<std::size_t> area{};
	boost::asio::thread_pool tp{Nthreads};

	// area of circle: c: r^2 * pi = pi
	// area of square: s: a*b = 1-(-1) * 1-(-1) = 4
	// ratio of areas: c/s = pi/4
	for(std::size_t i=0; i<N; ++i)
	{
		boost::asio::post(tp, [&gen, &area]() -> void
		{
			// generate random number is a [-1,1]*[-1,1] square
			t_real x = std::uniform_real_distribution<t_real>{-1, 1}(gen);
			t_real y = std::uniform_real_distribution<t_real>{-1, 1}(gen);

			// in unit circle?
			if(is_in_circle(1., x, y))
				++area;
		});
	}

	tp.join();
	return t_real(area)/t_real(N)*t_real(4);
}


template<class t_func>
t_real calc_area(t_real a, t_real b, t_func is_inside, std::size_t N, unsigned int Nthreads=4)
{
	std::mt19937_64 gen;
	gen.seed(g_seed);

	std::atomic<std::size_t> area{};
	boost::asio::thread_pool tp{Nthreads};

	// area of monte-carlo probed inner area: x
	// area of bounding rectangle: s: a*b
	// ratio of areas: x/s = x/(a*b)
	for(std::size_t i=0; i<N; ++i)
	{
		boost::asio::post(tp, [a, b, &gen, &area, &is_inside]() -> void
		{
			// generate random number is a [-a/2,a/2]*[-b/2,b/2] square
			t_real x = std::uniform_real_distribution<t_real>{-a/2., a/2.}(gen);
			t_real y = std::uniform_real_distribution<t_real>{-b/2., b/2.}(gen);

			// in inscribed area?
			if(is_inside(x,y))
				++area;
		});
	}

	tp.join();
	return t_real(area)/t_real(N) *a*b;
}


void plot_mandel(t_real start, t_real end, t_real delta, unsigned int Nthreads=4, unsigned int Niter=100)
{
	boost::asio::thread_pool tp{Nthreads};
	unsigned Npixels = unsigned((end-start) / delta);
	t_real* pixels = new t_real[Npixels * Npixels];

	for(t_real y=start; y<=end; y+=delta)
	{
		boost::asio::post(tp, [y, start, end, delta, Niter, Npixels, pixels]() -> void
		{
			for(t_real x=start; x<=end; x+=delta)
			{
				auto [isinside, norm] = is_in_mandel(2., Niter, x, y);
				if(!isinside)
					norm = 0.;
				else
					norm = std::sqrt(norm);

				unsigned pixX = (x-start) / delta;
				unsigned pixY = (y-start) / delta;

				if(pixY < Npixels && pixX < Npixels)
					pixels[pixY*Npixels + pixX] = norm;
			}
		});
	}

	tp.join();


	// plot "mandel.dat" using (($1/1000-0.5)*4):(($2/1000-0.5)*4):3 matrix with image
	std::ofstream ofstr("mandel.dat");
	for(unsigned y=0; y<Npixels; ++y)
	{
		for(unsigned x=0; x<Npixels; ++x)
			ofstr << pixels[y*Npixels + x] << " ";
		ofstr << "\n";
	}

	delete[] pixels;
}


int main()
{
	const std::size_t N = 100000;
	std::cout.precision(8);

	// ------------------------------------------------------------------------

	{	// maximum number of supported threads
		std::cout << "--------------------------------------------------------\n";
		const unsigned int Nthreads = std::thread::hardware_concurrency();
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		std::cout << "pi = " << calc_pi(N, Nthreads) << std::endl;

		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}

	{	// only 1 thread (here: much less overhead!)
		std::cout << "--------------------------------------------------------\n";
		const unsigned int Nthreads = 1;
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		std::cout << "pi = " << calc_pi(N, Nthreads) << std::endl;

		auto dur = std::chrono::duration<t_real>{t_clock::now() - starttime};
		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}


	// ------------------------------------------------------------------------


	// inscribed function
	auto inscribed = [](t_real x, t_real y) -> bool
	{
		//return is_in_circle(1., x, y);
		return std::get<0>(is_in_mandel(2., 100, x, y));
	};


	{	// maximum number of supported threads
		std::cout << "--------------------------------------------------------\n";
		const unsigned int Nthreads = std::thread::hardware_concurrency();
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		std::cout << "area = " << calc_area(4., 4., inscribed, N, Nthreads) << std::endl;

		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}

	{	// only 1 thread (here: much less overhead!)
		std::cout << "--------------------------------------------------------\n";
		const unsigned int Nthreads = 1;
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		std::cout << "area = " << calc_area(4., 4., inscribed, N, Nthreads) << std::endl;

		auto dur = std::chrono::duration<t_real>{t_clock::now() - starttime};
		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}


	// ------------------------------------------------------------------------


	{	// maximum number of supported threads
		std::cout << "--------------------------------------------------------\n";
		const unsigned int Nthreads = std::thread::hardware_concurrency();
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		std::cout << "Plotting mandel..." << std::endl;
		plot_mandel(-2., 2., 0.004, Nthreads, 100);

		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}

	{	// only 1 thread (slower than threaded version!)
		std::cout << "--------------------------------------------------------\n";
		const unsigned int Nthreads = 1;
		std::cout << "Using " << Nthreads << " thread(s)." << std::endl;

		auto starttime = t_clock::now();
		std::cout << "Plotting mandel..." << std::endl;
		plot_mandel(-2., 2., 0.004, Nthreads, 100);

		std::cout << "Duration: "
			<< std::chrono::duration<t_real>{t_clock::now() - starttime}.count()
			<< " s" << std::endl;
		std::cout << "--------------------------------------------------------\n";
	}

	return 0;
}
