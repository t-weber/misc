/**
 * scheduler comparison
 * @author Tobias Weber
 * @date 22-sep-20
 * @license see 'LICENSE.EUPL' file
 */

#include <iostream>
#include <list>
#include <vector>
#include <deque>
#include <tuple>
#include <memory>


// ----------------------------------------------------------------------------

struct Proc
{
	int pid;
	int remaining_time;

	int scheduled_time{};
};


class ISched
{
public:
	virtual ~ISched() = default;

	virtual void AddProcess(std::shared_ptr<Proc> proc) = 0;
	virtual const std::shared_ptr<Proc> Schedule() = 0;

	virtual const char* GetSchedName() const = 0;
};

// ----------------------------------------------------------------------------


class CoopFCFS : public ISched
{
public:
	virtual void AddProcess(std::shared_ptr<Proc> proc) override
	{
		m_procs.push_back(proc);
	}

	virtual const std::shared_ptr<Proc> Schedule() override
	{
		if(!m_procs.size())
			return nullptr;

		auto proc = m_procs.front();

		proc->scheduled_time = proc->remaining_time;
		proc->remaining_time = 0;

		m_procs.pop_front();
		return proc;
	}

	virtual const char* GetSchedName() const override
	{
		return "Coop_FCFS";
	}

private:
	std::list<std::shared_ptr<Proc>> m_procs{};
};


class CoopSJF : public ISched
{
public:
	virtual void AddProcess(std::shared_ptr<Proc> proc) override
	{
		m_procs.push_back(proc);
		std::stable_sort(m_procs.begin(), m_procs.end(), [](const auto& proc1, const auto& proc2) -> bool
		{
			return proc1->remaining_time < proc2->remaining_time;
		});
	}

	virtual const std::shared_ptr<Proc> Schedule() override
	{
		if(!m_procs.size())
			return nullptr;

		auto proc = m_procs.front();

		proc->scheduled_time = proc->remaining_time;
		proc->remaining_time = 0;

		m_procs.pop_front();
		return proc;
	}

	virtual const char* GetSchedName() const override
	{
		return "Coop_SJF";
	}

private:
	std::deque<std::shared_ptr<Proc>> m_procs{};
};


// ----------------------------------------------------------------------------


// zero types
template<class t_scheds>
void tst_sched(const std::index_sequence<>&)
{}

// one type
template<class t_scheds, std::size_t idx>
void tst_sched(const std::index_sequence<idx>&)
{
	using t_sched = typename std::tuple_element<idx, t_scheds>::type;
	ISched* sched = new t_sched();

	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=0, .remaining_time=10}));
	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=1, .remaining_time=20}));
	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=2, .remaining_time=30}));
	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=3, .remaining_time=10}));
	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=4, .remaining_time=1}));

	std::cout << "Scheduler: " << sched->GetSchedName() << std::endl;
	while(1)
	{
		auto nextproc = sched->Schedule();
		if(!nextproc)
			break;

		std::cout << "Scheduling process " << nextproc->pid << " for " << nextproc->scheduled_time << " time units, "
			<< "remaining process time: " << nextproc->remaining_time << "." << std::endl;
	}
	std::cout << std::endl;
	delete sched;
}

// n types
template<class t_scheds, std::size_t idx, std::size_t ...seq>
typename std::enable_if<sizeof...(seq), void>::type tst_sched(const std::index_sequence<idx, seq...>&)
{
	tst_sched<t_scheds, idx>(std::index_sequence<idx>());
	if constexpr(sizeof...(seq))
		tst_sched<t_scheds, seq...>(std::index_sequence<seq...>());
}


int main()
{
	using t_scheds = std::tuple<CoopFCFS, CoopSJF>;
	tst_sched<t_scheds>(std::make_index_sequence<std::tuple_size<t_scheds>::value>());

	return 0;
}
