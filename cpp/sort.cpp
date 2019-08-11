/**
 * sort test
 * @author Tobias Weber
 * @date 31-may-19
 * @license: see 'LICENSE.EUPL' file
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <experimental/iterator>


#ifdef __cpp_concepts
	#pragma message("Enabled concepts.")

	// comparison function concept
	template<class Cmp, class t_elem>
	concept bool is_cmp = requires(Cmp cmp, const t_elem& e0, const t_elem& e1)
	{
    	{ cmp(e0, e1) } -> bool;
	};
#endif



template<class t_elem, template<class...> class t_cont=std::vector, class Cmp>
void merge_sort(t_cont<t_elem>& cont, Cmp cmp)
#ifdef __cpp_concepts
	requires(is_cmp<Cmp, t_elem>)
#endif
{
	if(cont.size() <= 1)
		return;

	t_cont<t_elem> cont1, cont2;
	std::size_t n = cont.size();

	// split the container
	for(std::size_t idx=0; idx<n; ++idx)
	{
		if(idx < n/2)
			cont1.push_back(cont[idx]);
		else
			cont2.push_back(cont[idx]);
	}

	if(cont1.size() == 0 || cont2.size() == 0)
		return;


	// sort
	merge_sort(cont1, cmp);
	merge_sort(cont2, cmp);


	// merge the two containers
	std::size_t idx = 0;
	std::size_t idx1=0, idx2=0;

	while(1)
	{
		if(cmp(cont1[idx1], cont2[idx2]))
			cont[idx++] = cont1[idx1++];
		else
			cont[idx++] = cont2[idx2++];

		if(idx1 >= cont1.size() || idx2 >= cont2.size())
			break;
	}

	// remaining elements
	for(; idx1 < cont1.size(); ++idx1)
		cont[idx++] = cont1[idx1];
	for(; idx2 < cont2.size(); ++idx2)
		cont[idx++] = cont2[idx2];
}



template<class t_elem, template<class...> class t_cont=std::vector, class Cmp>
void quick_sort(t_cont<t_elem>& cont, Cmp cmp)
#ifdef __cpp_concepts
	requires(is_cmp<Cmp, t_elem>)
#endif
{
	t_cont<t_elem> contSmaller, contLarger;


	// split on first element and sort into two containers
	const t_elem* pPivot = &cont[0];
	for(const t_elem& elem : cont)
	{
		if(&elem == pPivot)
			continue;
		if(cmp(elem, *pPivot))
			contSmaller.push_back(elem);
		else
			contLarger.push_back(elem);
	}


	//auto joiner1 = std::experimental::make_ostream_joiner(std::cout, ", ");
	//for(const auto& elem : contSmaller)
	//	*joiner1++ = elem;
	//std::cout << std::endl;


	// sort the two containers with smaller and larger elements
	if(contSmaller.size() >= 2)
		quick_sort(contSmaller, cmp);
	if(contLarger.size() >= 2)
		quick_sort(contLarger, cmp);


	// insert sorted elements into original container
	t_elem pivot = *pPivot;
	std::size_t idx = 0;
	for(const t_elem& elem : contSmaller)
		cont[idx++] = elem;
	cont[idx++] = pivot;
	for(const t_elem& elem : contLarger)
		cont[idx++] = elem;
}



template<class t_elem, template<class...> class t_cont=std::vector, class Cmp>
void quick_sort_idx(const t_cont<t_elem>& cont, Cmp cmp, t_cont<std::size_t>& contIdx)
#ifdef __cpp_concepts
	requires(is_cmp<Cmp, t_elem>)
#endif
{
	if(!cont.size())
		return;

	if(!contIdx.size())
	{
		contIdx.resize(cont.size());
		std::iota(contIdx.begin(), contIdx.end(), 0);
	}


	t_cont<std::size_t> contSmaller, contLarger;

	// split on first element and sort into two containers
	std::size_t idxPivot = contIdx[0];
	for(std::size_t idx : contIdx)
	{
		if(idx == idxPivot)
			continue;
		if(cmp(cont[idx], cont[idxPivot]))
			contSmaller.push_back(idx);
		else
			contLarger.push_back(idx);
	}


	// sort the two containers with smaller and larger elements
	if(contSmaller.size() >= 2)
		quick_sort_idx<t_elem, t_cont, Cmp>(cont, cmp, contSmaller);
	if(contLarger.size() >= 2)
		quick_sort_idx<t_elem, t_cont, Cmp>(cont, cmp, contLarger);


	// insert sorted elements into original container
	std::size_t idx = 0;
	for(std::size_t elem : contSmaller)
		contIdx[idx++] = elem;
	contIdx[idx++] = idxPivot;
	for(std::size_t elem : contLarger)
		contIdx[idx++] = elem;
}


template<class t_elem, template<class...> class t_cont=std::vector>
t_cont<t_elem> rearrange(const t_cont<t_elem>& cont, const t_cont<std::size_t>& contIdx)
{
	t_cont<t_elem> contNew;

	for(std::size_t idx : contIdx)
		contNew.push_back(cont[idx]);

	return contNew;
}



template<class t_elem, template<class...> class t_cont=std::vector, class Cmp>
void bubble_sort(t_cont<t_elem>& cont, Cmp cmp)
#ifdef __cpp_concepts
	requires(is_cmp<Cmp, t_elem>)
#endif
{
	std::size_t iTotalSwaps = 0;

	while(1)
	{
		std::size_t iNumSwaps = 0;

		for(auto iter=cont.begin(); std::next(iter)!=cont.end(); std::advance(iter,1))
		{
			auto iter_next = std::next(iter);

			bool bSmaller = cmp(*iter, *iter_next);
			bool bLarger = cmp(*iter_next, *iter);
			// if larger (but not equal)
			if(!bSmaller && bLarger)
			{
				std::swap(*iter, *iter_next);
				++iNumSwaps;
			}
		}

		iTotalSwaps += iNumSwaps;

		// no more swaps in last iteration?
		if(iNumSwaps == 0)
			break;
	}

	//std::cout << "Total swaps: " << iTotalSwaps << std::endl;
}



template<class t_elem, template<class...> class t_cont=std::vector, class Cmp>
void insertion_sort(t_cont<t_elem>& cont, Cmp cmp)
#ifdef __cpp_concepts
	requires(is_cmp<Cmp, t_elem>)
#endif
{
	std::size_t iTotalSwaps = 0;

	for(auto iter=cont.begin(); iter!=cont.end(); std::advance(iter,1))
	{
		auto iter_next = std::next(iter);
		if(iter_next == cont.end())
			break;

		for(; iter_next!=cont.begin(); std::advance(iter_next, -1))
		{
			auto iter_prev = std::next(iter_next, -1);

			if(!cmp(*iter_prev, *iter_next))
			{
				//std::cout << "Swapping " << *iter_prev << " and " << *iter_next << std::endl;
				std::swap(*iter_prev, *iter_next);
				++iTotalSwaps;
			}
		}
	}

	//std::cout << "Total swaps: " << iTotalSwaps << std::endl;
}



int main()
{
	const std::vector<int> _vec{{ 4, 5, 9, 1, 7, 0, 3, 8, 2, 9, 6, 4,}};
	auto cmp = [](auto i0, auto i1) -> bool
	{
		return i0 < i1;
	};


	std::cout << "unsorted: ";
	auto joiner = std::experimental::make_ostream_joiner(std::cout, ", ");
	for(const auto& elem : _vec)
		*joiner++ = elem;
	std::cout << std::endl;


	{
		auto vec = _vec;

		std::sort(vec.begin(), vec.end(), cmp);

		std::cout << "std::sort: ";
		auto joiner = std::experimental::make_ostream_joiner(std::cout, ", ");
		for(const auto& elem : vec)
			*joiner++ = elem;
		std::cout << std::endl;
	}


	{
		auto vec = _vec;
		merge_sort(vec, cmp);

		std::cout << "merge sort: ";
		auto joiner = std::experimental::make_ostream_joiner(std::cout, ", ");
		for(const auto& elem : vec)
			*joiner++ = elem;
		std::cout << std::endl;
	}


	{
		auto vec = _vec;
		quick_sort(vec, cmp);

		std::cout << "quick sort: ";
		auto joiner = std::experimental::make_ostream_joiner(std::cout, ", ");
		for(const auto& elem : vec)
			*joiner++ = elem;
		std::cout << std::endl;
	}


	{
		auto vec = _vec;
		std::vector<std::size_t> indices;
		quick_sort_idx(vec, cmp, indices);
		vec = rearrange(vec, indices);

		std::cout << "quick sort (indices): ";
		auto joiner = std::experimental::make_ostream_joiner(std::cout, ", ");
		for(const auto& elem : vec)
			*joiner++ = elem;
		std::cout << std::endl;
	}


	{
		auto vec = _vec;
		bubble_sort(vec, cmp);

		std::cout << "bubble sort: ";
		auto joiner = std::experimental::make_ostream_joiner(std::cout, ", ");
		for(const auto& elem : vec)
			*joiner++ = elem;
		std::cout << std::endl;
	}


	{
		auto vec = _vec;
		insertion_sort(vec, cmp);

		std::cout << "insertion sort: ";
		auto joiner = std::experimental::make_ostream_joiner(std::cout, ", ");
		for(const auto& elem : vec)
			*joiner++ = elem;
		std::cout << std::endl;
	}

	return 0;
}
