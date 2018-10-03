/**
 * mpi test
 * @author Tobias Weber
 * @date 3-oct-18
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_68_0/doc/html/mpi/tutorial.html
 *  * https://github.com/boostorg/mpi/tree/develop/example
 *
 * build: /usr/lib64/openmpi/bin/mpic++ -std=c++17 -o mpi mpi.cpp -lboost_mpi
 * run: /usr/lib64/openmpi/bin/mpirun -np 4 ./mpi
 */

#include <boost/mpi.hpp>
#include <iostream>


void send_tst(boost::mpi::communicator& mpi)
{
	// index of this process
	const std::size_t mpi_idx = mpi.rank();

	// sending messages
	if(mpi_idx == 0)
	{
		// send value to proc 1
		mpi.send<double>(1 /*destination index*/, 0 /*tag*/, 12.34 /*value*/);

		// send vector to proc 2
		mpi.send(2 /*destination index*/, 1 /*tag*/, std::vector<int>{{0,1,2,3}} /*value*/);
	}
	else if(mpi_idx == 1)
	{
		// receive value
		double d;
		mpi.recv<double>(0 /*source index*/, 0 /*tag*/, d);
		std::cout << "idx: " << mpi_idx <<  ", val: " << d << std::endl;
	}
	else if(mpi_idx == 2)
	{
		// receive vector
		std::vector<int> vec;
		mpi.recv(0 /*source index*/, 1 /*tag*/, vec);
		std::cout << "idx: " <<  mpi_idx <<  ", val: ";
		for(const auto& v : vec) std::cout << v << " ";
		std::cout << std::endl;
	}
}


void scatter_tst(boost::mpi::communicator& mpi)
{
	// index of this process
	const std::size_t mpi_idx = mpi.rank();

	// scatter collective
	double d;
	boost::mpi::scatter<double>(mpi, std::vector<double>{{5., 4., 3., 2.}}, d, 0 /*scattered from proc 0*/);
	std::cout << "scattered to proc " << mpi_idx << ": " << d << std::endl;
}


void gather_tst(boost::mpi::communicator& mpi)
{
	// index of this process
	const std::size_t mpi_idx = mpi.rank();

	// gather collective
	if(mpi_idx == 0)
	{
		std::vector<double> vec;
		boost::mpi::gather<double>(mpi, 1.23 /*value*/, vec /*final storage*/, 0 /*index of collecting process*/);

		std::cout << "gathered: ";
		for(const auto& d : vec) std::cout << d << " ";
		std::cout << std::endl;
	}
	else if(mpi_idx == 1)
	{
		boost::mpi::gather<double>(mpi, 2.34, 0 /*to process 0*/);
	}
	else if(mpi_idx == 2)
	{
		boost::mpi::gather<double>(mpi, 3.45, 0 /*to process 0*/);
	}
	else if(mpi_idx == 3)
	{
		boost::mpi::gather<double>(mpi, 4.56, 0 /*to process 0*/);
	}
}



int main(int argc, char** argv)
{
	boost::mpi::environment mpi_env(argc, argv);
	boost::mpi::communicator mpi;

	const std::size_t mpi_size = mpi.size();	// number of processes
	//const std::size_t mpi_idx = mpi.rank();	// index of this process

	if(mpi_size != 4)
	{
		std::cerr << "Need four processes." << std::endl;
		return -1;
	}


	send_tst(mpi);
	mpi.barrier();

	scatter_tst(mpi);
	mpi.barrier();

	gather_tst(mpi);
	mpi.barrier();

	return 0;
}
