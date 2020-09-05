/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 28-mar-19
 * @license see 'LICENSE.EUPL' file
 * @see https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
 *
 * g++ -std=c++17 -o producer producer_mon.cpp -lpthread
 */

#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>


#define MAX_ELEMS 5


template<class t_elem>
class ListMonitor
{
public:
	void put(const t_elem& elem)
	{
#if MAX_ELEMS > 0
		std::unique_lock _ul{m_max_elem_access};
		m_max_elems.wait(_ul, [this]()->bool { return m_lst.size() < MAX_ELEMS; });
#endif

		std::scoped_lock _sl{m_elem_access};
		m_lst.push_back(elem);

		m_has_elems.notify_one();

		std::cout << "Inserted " << elem << ", number of elements now: " << m_lst.size() << std::endl;
	}


	t_elem get()
	{
		std::unique_lock _ul{m_elem_access};
		m_has_elems.wait(_ul, [this]()->bool { return m_lst.size() > 0; });

		//std::scoped_lock _sl{m_max_elem_access};
		t_elem elem = std::move(*m_lst.begin());
		m_lst.pop_front();

#if MAX_ELEMS > 0
		m_max_elems.notify_one();
#endif

		std::cout << "Removed " << elem << ", number of elements now: " << m_lst.size() << std::endl;
		return elem;
	}


private:
	std::list<t_elem> m_lst{};

	std::mutex m_elem_access{};
	std::condition_variable m_has_elems{};

#if MAX_ELEMS > 0
	std::mutex m_max_elem_access{};
	std::condition_variable m_max_elems{};
#endif
};



ListMonitor<int> g_lstmon;


void produce()
{
	int i = 0;

	while(1)
	{
		g_lstmon.put(i++);
//		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}


void consume()
{
	while(1)
	{
		int i = g_lstmon.get();
//		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
}



int main()
{
	std::thread prod{&produce};
	std::thread cons{&consume};

	prod.join();
	cons.join();

	return 0;
}
