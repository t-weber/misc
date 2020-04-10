/**
 * maximum subvector algo test
 * @author Tobias Weber
 * @date 10-apr-20
 * @license see 'LICENSE.EUPL' file
 *
 * @see https://en.wikipedia.org/wiki/Maximum_subarray_problem
 */

#include <iostream>
#include <iomanip>
#include <array>
#include <algorithm>
#include <random>
#include <type_traits>
#include <cstdint>


//#define DO_TEST


template<class t_num>
t_num get_rand()
{
	static std::mt19937 rng{std::random_device{}()};

	t_num min = -std::numeric_limits<t_num>::max();
	t_num max = std::numeric_limits<t_num>::max();

	if constexpr(std::is_integral_v<t_num>)
		return std::uniform_int_distribution<t_num>(min, max)(rng);
	else
		return std::uniform_real_distribution<t_num>(min, max)(rng);
}



template<class t_largernum, class t_arr>
std::tuple<std::size_t, std::size_t, t_largernum>
subvec_ineffic(const t_arr& arr)
{
	std::size_t start_idx = 0;
	std::size_t end_idx = 0;
	t_largernum val = -std::numeric_limits<t_largernum>::max();

	for(std::size_t start=0; start<arr.size(); ++start)
	{
		for(std::size_t end=start+1; end<=arr.size(); ++end)
		{
			t_largernum sum = std::accumulate(arr.begin()+start, arr.begin()+end, t_largernum{0});
			if(sum > val)
			{
				val = sum;
				start_idx = start;
				end_idx = end;
			}
		}
	}

	return std::make_tuple(start_idx, end_idx, val);
}



template<class t_largernum, class t_arr>
std::tuple<std::size_t, std::size_t, t_largernum>
subvec_sweep(const t_arr& arr)
{
	std::size_t cached_start_idx = 0;
	std::size_t start_idx = 0;
	std::size_t end_idx = 0;

	t_largernum newval = 0;
	t_largernum val = 0;

	for(std::size_t idx=0; idx<arr.size(); ++idx)
	{
		if(newval < -arr[idx])
		{
			newval = t_largernum{0};
			cached_start_idx = idx+1;
		}
		else
		{
			newval += arr[idx];
		}

		if(newval > val)
		{
			val = newval;
			start_idx = cached_start_idx;
			end_idx = idx+1;
		}
	}

	return std::make_tuple(start_idx, end_idx, val);
}



#ifdef DO_TEST

int main()
{
	using t_num = std::int32_t;
	using t_largernum = std::int64_t;
	constexpr std::size_t N = 256;


	for(int i=0; i<1000; ++i)
	{
		std::cout << "\rRun " << i << "   ...   ";
		std::array<t_num, N> deltas;
		std::generate(deltas.begin(), deltas.end(), []() -> t_num
		{
			return get_rand<t_num>();
		});


		auto tup1 = subvec_ineffic<t_largernum>(deltas);
		auto tup2 = subvec_sweep<t_largernum>(deltas);

		if(tup1 == tup2)
		{
			std::cout << "OK" << std::endl;
		}
		else
		{
			std::cerr << "Mismatch: "
				<< "1: " << std::get<0>(tup1) << ", " << std::get<1>(tup1) << ", " << std::get<2>(tup1) << "\n"
				<< "2: " << std::get<0>(tup2) << ", " << std::get<1>(tup2) << ", " << std::get<2>(tup2) << std::endl;
			break;
		}
	}

	return 0;
}

#else

int main()
{
	using t_num = std::int8_t;
	using t_largernum = std::int64_t;
	constexpr std::size_t N = 128;

	std::array<t_num, N> deltas;
	std::generate(deltas.begin(), deltas.end(), []() -> t_num
	{
		return get_rand<t_num>();
	});


	for(std::size_t i=0; i<deltas.size(); ++i)
		std::cout << +deltas[i] << " ";
	std::cout << std::endl;


	{
		auto [start_idx, end_idx, maxval] = subvec_ineffic<t_largernum>(deltas);
		std::cout << "Max. subvec range: [" << start_idx << ", " << end_idx << "[, sum: " << +maxval << std::endl;
	}

	{
		auto [start_idx, end_idx, maxval] = subvec_sweep<t_largernum>(deltas);
		std::cout << "Max. subvec range: [" << start_idx << ", " << end_idx << "[, sum: " << +maxval << std::endl;
	}

	return 0;
}

#endif
