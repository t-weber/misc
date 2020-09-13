/**
 * producer/consumer test
 * @author Tobias Weber
 * @date 28-mar-19
 * @license see 'LICENSE.EUPL' file
 *
 * g++ -std=c++17 -o ringbuffer ringbuffer.cpp -lpthread
 */

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>

#if __has_include(<semaphore>)
	#pragma message("Using standard semaphore.")
	#include <semaphore>

	template<std::ptrdiff_t maxval> using t_sema = std::counting_semaphore<maxval>;
#else
	#pragma message("Using custom semaphore.")

	/**
 	 * simple semaphore
 	*/
	class Sema
	{
	public:
		Sema(int ctr) : m_ctr{ctr} {}

		void acquire()
		{
			std::unique_lock _ul{m_mtxcond};
			m_cond.wait(_ul, [this]()->bool { return m_ctr > 0; });

			--m_ctr;
		}

		void release()
		{
			++m_ctr;

			// lock mutex in case of spurious release of wait() in acquire()
			std::scoped_lock _sl{m_mtxcond};
			m_cond.notify_one();
		}

	private:
		std::atomic<int> m_ctr{0};
		std::condition_variable m_cond{};
		std::mutex m_mtxcond{};
	};

	template<std::ptrdiff_t> using t_sema = Sema;
#endif



template<class t_elem>
class RingBuffer
{
public:
	RingBuffer(std::size_t num_slots)
		: m_buf(num_slots, t_elem{}),
		m_sem_remaining_slots{static_cast<int>(num_slots)}
	{}


	void put(const t_elem& elem)
	{
		m_sem_remaining_slots.acquire();

		m_sem_access.acquire();
			m_buf[m_idx_put] = elem;
			advance_index(&m_idx_put);
		m_sem_access.release();

		m_sem_used_slots.release();
	}


	t_elem get()
	{
		m_sem_used_slots.acquire();

		m_sem_access.acquire();
			t_elem elem = m_buf[m_idx_get];
			advance_index(&m_idx_get);
		m_sem_access.release();

		m_sem_remaining_slots.release();
		return elem;
	}


	std::size_t num_slots() const
	{
		return m_buf.size();
	}


protected:
	void advance_index(std::size_t *idx) const
	{
		++*idx;
		*idx %= num_slots();
	}


private:
	std::vector<t_elem> m_buf;

	t_sema<16> m_sem_access{1};
	t_sema<16> m_sem_remaining_slots{10};
	t_sema<16> m_sem_used_slots{0};

	std::size_t m_idx_put{0};
	std::size_t m_idx_get{0};
};



std::mutex g_mtx_cout;

void produce(RingBuffer<int>* buf)
{
	int i = 0;

	while(1)
	{
		buf->put(i++);

		{
			std::lock_guard<std::mutex> _lck(g_mtx_cout);
			std::cout << "Inserted " << (i-1) << "." << std::endl;
		}

//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}


void consume(RingBuffer<int>* buf)
{
	while(1)
	{
		int i = buf->get();

		{
			std::lock_guard<std::mutex> _lck(g_mtx_cout);
			std::cout << "Removed " << i << "." << std::endl;
		}

//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}



int main()
{
	RingBuffer<int> buf(10);

	std::thread prod{&produce, &buf};
	std::thread cons{&consume, &buf};

	prod.join();
	cons.join();

	return 0;
}
