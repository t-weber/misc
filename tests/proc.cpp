/**
 * process test
 * @author Tobias Weber
 * @date 5-apr-20
 * @license: see 'LICENSE.GPL' file
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include <unistd.h>


class A
{
private:
	std::shared_ptr<int> ptr = std::make_shared<int>(0);
	pid_t pid = 0;

public:
	A()
	{
		std::cout << "In " << __func__ << std::endl;

		pid = fork();
		if(pid < 0)
		{
			std::cerr << "Fork failed." << std::endl;
		}
		else if(pid == 0)
		{
			*ptr = 123;
			std::cout << "In child process " << getpid() << "." << std::endl;
			//std::exit(0);	// no destructors called for child process.
			//std::abort();
		}
		else
		{
			*ptr = 987;
			std::cout << "In main process " << getpid()
				<< ": Spawned child process with id " << pid << "." << std::endl;
		}
	}


	int get_pid() const { return pid; }
	int data() const { return *ptr; }
	void* pointer() const { return reinterpret_cast<void*>(ptr.get()); }


	~A()
	{
		std::cout << "Process " << getpid() << ": In " << __func__ << std::endl;
		std::cout << "use_count: " << ptr.use_count() << std::endl;
	}
};


int main()
{
	A a{};

	for(int i=0; i<100; ++i)
	{
		// Each process has their own copy of the data.
		// The pointers seemingly point to the same address, but
		// are actually in a different process space.
		std::cout << "pid: " << std::dec << a.get_pid();
		std::cout << ", ptr: " << std::hex << a.pointer();
		std::cout << ", data: " << std::dec << a.data();
		std::cout << "." << std::endl;
	}

	return 0;
}
