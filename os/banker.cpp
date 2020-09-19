/**
 * deadlock avoidance algo
 * @author Tobias Weber
 * @date 19-sep-2020
 * @license see 'LICENSE.EUPL' file
 *
 * @desc see: https://en.wikipedia.org/wiki/Banker's_algorithm
 *
 * g++ -std=c++20 -o banker banker.cpp
 */

#include <iostream>
#include <vector>
#include <tuple>

#include "../libs/math_conts.h"
#include "../libs/math_algos.h"

//using namespace m_ops;


using t_real = double;
using t_vec = m::vec<t_real, std::vector>;
using t_mat = m::mat<t_real, std::vector>;


// [ deadlock?, process termination sequence ]
std::tuple<bool, std::vector<std::size_t>>
banker(t_vec avail_res, const t_mat& max_alloc, const t_mat& cur_alloc)
{
	const std::size_t num_res = max_alloc.size1();
	const std::size_t num_procs = max_alloc.size2();

	const t_mat needed_alloc = max_alloc - cur_alloc;
	//std::cout << "needed alloc: " << needed_alloc << std::endl;

	std::vector<std::size_t> termination_seq;

	while(true)
	{
		std::size_t num_terminated = 0;

		for(std::size_t proc=0; proc<num_procs; ++proc)
		{
			// has this proc already terminated?
			if(std::find(termination_seq.begin(), termination_seq.end(), proc) != termination_seq.end())
				continue;

			bool can_alloc = true;
			for(std::size_t res=0; res<num_res; ++res)
			{
				if(needed_alloc(res, proc) > avail_res[res])
				{
					can_alloc = false;
					break;
				}
			}

			if(can_alloc)
			{
				// free resources of proc
				avail_res += m::col<t_mat, t_vec>(cur_alloc, proc);
				termination_seq.push_back(proc);

				++num_terminated;
			}
		}

		// no more changes or all terminated
		if(num_terminated==0 || termination_seq.size()==num_procs)
			break;
	}

	return std::make_tuple(termination_seq.size()<num_procs, termination_seq);
}


int main()
{
	// remaining system resources
	t_vec avail_res = m::create<t_vec>({1, 2, 3});

	t_mat max_alloc = m::create<t_mat>({
		{3, 2, 3},	/* maximum resources of proc 0 */
		{4, 2, 3},	/* maximum resources of proc 1 */
		{5, 2, 3},	/* maximum resources of proc 2 */
		{1, 2, 3},	/* maximum resources of proc 3 */
	});

	t_mat cur_alloc = m::create<t_mat>({
		{1, 2, 3},	/* current resources of proc 0 */
		{1, 2, 3},	/* current resources of proc 1 */
		{1, 2, 3},	/* current resources of proc 2 */
		{1, 2, 3},	/* current resources of proc 3 */
	});

	auto [deadlock, termination_seq] = banker(avail_res, max_alloc, cur_alloc);

	std::cout << "Deadlock: " << std::boolalpha << deadlock << "." << std::endl;

	std::cout << "Process termination sequence: ";
	for(std::size_t proc : termination_seq)
		std::cout << proc << ", ";
	std::cout << std::endl;

	return 0;
}
