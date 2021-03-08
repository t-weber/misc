/**
 * interprocess tests
 * @author Tobias Weber
 * @date 9-apr-2020
 * @license: see 'LICENSE.EUPL' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_72_0/doc/html/interprocess.html
 *  * https://www.boost.org/doc/libs/1_72_0/doc/html/interprocess/quick_guide.html
 *  * https://www.boost.org/doc/libs/1_72_0/doc/html/interprocess/allocators_containers.html
 *  * https://theboostcpplibraries.com/boost.interprocess
 *
 * g++ -std=c++17 -o interproc interproc.cpp -lboost_system -lrt -lpthread
 */

#include <iostream>
#include <string>

//#include <unordered_map>	// DOES NOT WORK in shared memory
#include <boost/unordered_map.hpp>

#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/allocators/node_allocator.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef BOOST_INTERPROCESS_XSI_SHARED_MEMORY_OBJECTS
	#pragma message("Boost.Interprocess shares memory objects via XSI.")
#endif

#ifdef BOOST_INTERPROCESS_POSIX_SHARED_MEMORY_OBJECTS
	#pragma message("Boost.Interprocess shares memory objects via Posix.")
#endif

#ifdef BOOST_INTERPROCESS_FILESYSTEM_BASED_POSIX_SHARED_MEMORY
	#pragma message("Boost.Interprocess uses shared memory via the filesystem.")
#endif

#ifdef BOOST_INTERPROCESS_RUNTIME_FILESYSTEM_BASED_POSIX_SHARED_MEMORY
	#pragma message("Boost.Interprocess uses shared memory via the runtime filesystem.")
#endif

namespace interp = boost::interprocess;
namespace pt = boost::posix_time;


// allocator for an object in shared memory
template<class t_obj>
//using t_shared_allocator = interp::allocator<t_obj, interp::managed_shared_memory::segment_manager>;
using t_shared_allocator = interp::node_allocator<t_obj, interp::managed_shared_memory::segment_manager>;


// am unordered map in shared memory
template<class t_key, class t_val>
using t_sharedmap = boost::unordered_map<t_key, t_val, std::hash<t_key>, std::equal_to<t_key>,
	t_shared_allocator<std::pair<const t_key, t_val>>>;

// another map in shared memory
template<class t_key, class t_val>
using t_sharedmap2 = interp::map<t_key, t_val, std::less<t_key>,
	t_shared_allocator<std::pair<const t_key, t_val>>>;


// a vector in shared memory
template<class T> using t_sharedvector = interp::vector<T, t_shared_allocator<T>>;


using t_map = t_sharedmap<int, double>;
using t_map2 = t_sharedmap2<int, double>;
using t_vector = t_sharedvector<double>;


void server()
{
	std::cout << "Starting server..." << std::endl;

	try
	{
		// create shared memory (accessible under /dev/shm/test_interp)
		interp::managed_shared_memory mem{interp::create_only, "test_interproc", 128*1024};

		{
			// create a map in shared memory
			t_map::allocator_type alloc{mem.get_segment_manager()};
			t_map::hasher hash{};
			t_map::key_equal equal{};

			auto map = mem.construct<t_map>("map")(16, hash, equal, alloc);
			std::cout << "address: " << std::hex << (void*)map << std::endl;

			(*map)[123] = 456.78;
			(*map)[987] = 111.22;

			for(const auto& pair : *map)
				std::cout << std::dec << pair.first << " -> " << pair.second << std::endl;

			std::cout << "Free shared memory: " << mem.get_free_memory() << " / " << mem.get_size() << std::endl;
		}

		if(0)	// DOES NOT WORK, crashes client!
		{
			// create a map in shared memory
			t_map2::allocator_type alloc{mem.get_segment_manager()};
			t_map2::key_compare comp{};

			auto map = mem.construct<t_map2>("map2")(comp, alloc);
			std::cout << "address: " << std::hex << (void*)map << std::endl;

			map->insert(std::make_pair(123, 456.78));
			map->insert(std::make_pair(567, 98.76));

			for(const auto& pair : *map)
				std::cout << std::dec << pair.first << " -> " << pair.second << std::endl;

			std::cout << "Free shared memory: " << mem.get_free_memory() << " / " << mem.get_size() << std::endl;
		}

		{
			// create a vector in shared memory
			t_vector::allocator_type alloc{mem.get_segment_manager()};

			auto vector = mem.construct<t_vector>("vector")(alloc);
			std::cout << "address: " << std::hex << (void*)vector << std::endl;

			vector->push_back(123.45);
			vector->push_back(456.98);

			for(const auto& item : *vector)
				std::cout << std::dec << item << std::endl;

			std::cout << "Free shared memory: " << mem.get_free_memory() << " / " << mem.get_size() << std::endl;
		}

		{
			// create an array
			// the object is unnamed when using the allocator directly...
			//t_shared_allocator<int> alloc{mem.get_segment_manager()};
			//auto arr = alloc.allocate(4);

			auto arr = mem.construct<int>("arr")[4](0);

			for(int i=0; i<4; ++i)
				arr[i] = i*i;

			std::cout << "Free shared memory: " << mem.get_free_memory() << " / " << mem.get_size() << std::endl;
		}

		{
			// sending messages
			using t_msg = std::tuple<int, double>;
			interp::message_queue msg{interp::create_only, "test_msg", 4, sizeof(t_msg)};

			for(int i=0; i<3; ++i)
			{
				t_msg tup = std::make_tuple(i, i*i);

				std::cout << "Sending message " << i << " with size " << sizeof(tup)
					<< ": " << std::get<0>(tup) << ", " << std::get<1>(tup) << "..." << std::endl;

				// blocks if queue is full
				//msg.send(&tup, sizeof(tup), 0);
				pt::ptime ti = pt::second_clock::local_time() + pt::seconds(1);
				//std::cout << ti << std::endl;
				if(!msg.timed_send(&tup, sizeof(tup), 0, ti))
					std::cerr << "\tError sending message " << i << "." << std::endl;
			}

			for(int i=3; i<5; ++i)
			{
				t_msg tup = std::make_tuple(i, i*i);

				std::cout << "Sending message " << i << " with size " << sizeof(tup)
					<< ": " << std::get<0>(tup) << ", " << std::get<1>(tup) << "..." << std::endl;

				// fails if queue is full
				if(!msg.try_send(&tup, sizeof(tup), 0))
					std::cerr << "\tError sending message " << i << "." << std::endl;
			}

			for(int i=6; i<10; ++i)
			{
				t_msg tup = std::make_tuple(i, i*i);

				std::cout << "Sending message " << i << " with size " << sizeof(tup)
				<< ": " << std::get<0>(tup) << ", " << std::get<1>(tup) << "..." << std::endl;

				// fails if queue is full
				pt::ptime ti = pt::microsec_clock::universal_time() + pt::milliseconds(500);
				if(!msg.timed_send(&tup, sizeof(tup), 0, ti))
					std::cerr << "\tError sending message " << i << "." << std::endl;
			}

			std::cout << "Free shared memory: " << mem.get_free_memory() << " / " << mem.get_size() << std::endl;
		}

		if(0)
		{
			// /dev/shm/test_interp2
			interp::managed_shared_memory mem{interp::create_only, "test_interproc2", 1024};
			//char* pChar = static_cast<char*>(mem.allocate_aligned(64, 8));
			char* pChar = static_cast<char*>(mem.allocate(128));

			interp::bufferstream sstr(pChar, 64);
			sstr << "Test " << 123 << " " << "56.78" << std::endl;

			mem.deallocate(pChar);
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}


void client()
{
	std::cout << "Starting client..." << std::endl;

	try
	{
		// open shared memory
		interp::managed_shared_memory mem{interp::open_only, "test_interproc"};
		std::cout << "Shared memory ok: " << mem.check_sanity() << std::endl;
		std::cout << mem.get_num_named_objects() << " objects in shared memory." << std::endl;

		{
			// find map
			auto [map, size] = mem.find<t_map>("map");
			std::cout << "map address: " << std::hex << (void*)map << ", size: " << std::dec << size << std::endl;
			if(map)
			{
				for(const auto& pair : *map)
					std::cout << std::dec << pair.first << " -> " << pair.second << std::endl;
			}
			else
			{
				std::cerr << "Shared map not found." << std::endl;
			}
		}

		{
			// find map
			auto [map, size] = mem.find<t_map>("map2");
			std::cout << "map2 address: " << std::hex << (void*)map << ", size: " << std::dec << size << std::endl;
			if(map)
			{
				for(const auto& pair : *map)
					std::cout << std::dec << pair.first << " -> " << pair.second << std::endl;
			}
			else
			{
				std::cerr << "Shared map not found." << std::endl;
			}
		}

		{
			// find vector
			auto [vector, size] = mem.find<t_vector>("vector");
			std::cout << "vector address: " << std::hex << (void*)vector << ", size: " << std::dec << size << std::endl;
			if(vector)
			{
				for(const auto& item : *vector)
					std::cout << std::dec << item << std::endl;
			}
			else
			{
				std::cerr << "Shared vector not found." << std::endl;
			}
		}

		{
			// find array
			auto [arr, size] = mem.find<int>("arr");
			std::cout << "array address: " << std::hex << (void*)arr << ", size: " << std::dec << size << std::endl;
			if(arr)
			{
				for(int i=0; i<4; ++i)
					std::cout << std::dec << arr[i] << std::endl;
			}
			else
			{
				std::cerr << "Shared array not found." << std::endl;
			}
		}

		{
			// receiving messages
			using t_msg = std::tuple<int, double>;
			interp::message_queue msg{interp::open_only, "test_msg"};

			for(int i=0; i<10; ++i)
			{
				t_msg tup;

				std::size_t received = 0;
				unsigned int priority = 0;

				// blocks if queue is full
				//msg.receive(&tup, sizeof(tup), received, priority);

				// fails if queue is full
				//if(!msg.try_receive(&tup, sizeof(tup), received, priority))
				//	std::cerr << "Error receiving message " << i << "." << std::endl;

				pt::ptime ti = pt::microsec_clock::universal_time() + pt::milliseconds(500);
				if(!msg.timed_receive(&tup, sizeof(tup), received, priority, ti))
					std::cerr << "Error receiving message " << i << "." << std::endl;
				else
					std::cout << "Received message " << i << " with size " << received
						<< ": " << std::get<0>(tup) << ", " << std::get<1>(tup) << "." << std::endl;
			}
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}


void cleanup()
{
	std::cout << "Cleaning up..." << std::endl;

	try
	{
		//interp::managed_shared_memory mem(interp::open_only, "test_interproc");
		//mem.destroy<t_map>("map");
		//mem.destroy<t_map>("map2");
		//mem.destroy<t_map>("vector");
		//mem.destroy<t_map>("arr");

		interp::message_queue::remove("test_msg");
		interp::shared_memory_object::remove("test_interproc");
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}


int main(int argc, char** argv)
{
	if(argc <= 1)
	{
		cleanup();
		server();
	}
	else
	{
		client();
		cleanup();
	}

	return 0;
}
