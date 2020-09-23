/**
 * scheduler comparison
 * @author Tobias Weber
 * @date 22-sep-20
 * @license see 'LICENSE.EUPL' file
 */

#include <iostream>
#include <algorithm>
#include <list>
#include <vector>
#include <deque>
#include <tuple>
#include <memory>


// ----------------------------------------------------------------------------

#define DEFAULT_TIMESLICE 4


struct Proc
{
	int pid;
	unsigned int remaining_time;

	unsigned int scheduled_time{};
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


/**
 * preemptive version of FCFS
 */
class PreemptRR : public ISched
{
public:
	PreemptRR(unsigned int timeslice=DEFAULT_TIMESLICE) : m_timeslice{timeslice}
	{}

	virtual void AddProcess(std::shared_ptr<Proc> proc) override
	{
		m_procs.push_back(proc);
	}

	virtual const std::shared_ptr<Proc> Schedule() override
	{
		if(!m_procs.size())
			return nullptr;

		auto proc = m_procs.front();

		proc->scheduled_time = std::min(proc->remaining_time, m_timeslice);
		proc->remaining_time -= proc->scheduled_time;

		m_procs.pop_front();
		if(proc->remaining_time > 0)
			m_procs.push_back(proc);

		return proc;
	}

	virtual const char* GetSchedName() const override
	{
		return "Preempt_RR";
	}

private:
	std::list<std::shared_ptr<Proc>> m_procs{};
	unsigned int m_timeslice{};
};


/**
 * preemptive version of SFJ
 */
class PreemptSRTF : public ISched
{
public:
	PreemptSRTF(unsigned int timeslice=DEFAULT_TIMESLICE) : m_timeslice{timeslice}
	{}

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

		proc->scheduled_time = std::min(proc->remaining_time, m_timeslice);
		proc->remaining_time -= proc->scheduled_time;

		m_procs.pop_front();
		if(proc->remaining_time > 0)
		{
			m_procs.push_back(proc);

			// re-sort all others, except the one which was just running
			if(m_procs.size() > 1)
			{
				int skip_end = 1;	// skip_end=0 => same as SJF
				std::stable_sort(m_procs.begin(), std::prev(m_procs.end(),skip_end), [](const auto& proc1, const auto& proc2) -> bool
				{
					return proc1->remaining_time < proc2->remaining_time;
				});
			}
		}

		return proc;
	}

	virtual const char* GetSchedName() const override
	{
		return "PreemptSRTF";
	}

private:
	std::deque<std::shared_ptr<Proc>> m_procs{};
	unsigned int m_timeslice{};
};


// ----------------------------------------------------------------------------


template<class t_scheds, std::size_t idx>
void tst_sched()
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

template<class t_scheds, std::size_t ...seq>
typename std::enable_if<sizeof...(seq)!=0, void>::type tst_sched(const std::index_sequence<seq...>&)
{
	( tst_sched<t_scheds, seq>(), ... );
}


int main()
{
	using t_scheds = std::tuple<CoopFCFS, CoopSJF, PreemptRR, PreemptSRTF>;
	tst_sched<t_scheds>(std::make_index_sequence<std::tuple_size<t_scheds>::value>());

	return 0;
}
