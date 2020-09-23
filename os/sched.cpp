/**
 * scheduler comparison
 * @author Tobias Weber
 * @date 22-sep-20
 * @license see 'LICENSE.EUPL' file
 *
 * @see https://en.wikipedia.org/wiki/Scheduling_(computing)
 */

#include <iostream>
#include <algorithm>
#include <list>
#include <vector>
#include <deque>
#include <tuple>
#include <memory>


// ----------------------------------------------------------------------------

#define DEFAULT_PREEMPT_TIMESLICE 5


struct Proc
{
	int pid{};
	unsigned int remaining_time{};
	unsigned int prio{};

	unsigned int scheduled_time{};
	double virtual_time{};
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


class CoopPrio : public ISched
{
public:
	virtual void AddProcess(std::shared_ptr<Proc> proc) override
	{
		m_procs.push_back(proc);
		std::stable_sort(m_procs.begin(), m_procs.end(), [](const auto& proc1, const auto& proc2) -> bool
		{
			return proc1->prio >= proc2->prio;
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
		return "Coop_Prio";
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
	PreemptRR(unsigned int timeslice=DEFAULT_PREEMPT_TIMESLICE) : m_timeslice{timeslice}
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
	PreemptSRTF(unsigned int timeslice=DEFAULT_PREEMPT_TIMESLICE) : m_timeslice{timeslice}
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


/**
 * preemptive version of priority scheduling
 */
class PreemptPrio : public ISched
{
public:
	PreemptPrio(unsigned int timeslice=DEFAULT_PREEMPT_TIMESLICE) : m_timeslice{timeslice}
	{}

	virtual void AddProcess(std::shared_ptr<Proc> proc) override
	{
		m_procs.push_back(proc);
		std::stable_sort(m_procs.begin(), m_procs.end(), [](const auto& proc1, const auto& proc2) -> bool
		{
			return proc1->prio >= proc2->prio;
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
				int skip_end = 1;	// skip_end=0 => same as cooperative prio.
				std::stable_sort(m_procs.begin(), std::prev(m_procs.end(),skip_end), [](const auto& proc1, const auto& proc2) -> bool
				{
					return proc1->prio >= proc2->prio;
				});
			}
		}

		return proc;
	}

	virtual const char* GetSchedName() const override
	{
		return "PreemptPrio";
	}

private:
	std::deque<std::shared_ptr<Proc>> m_procs{};
	unsigned int m_timeslice{};
};


/**
 * preemptive version of priority scheduling
 */
class PreemptCFS : public ISched
{
public:
	PreemptCFS(unsigned int timeslice=DEFAULT_PREEMPT_TIMESLICE) : m_timeslice{timeslice}
	{}

	virtual void AddProcess(std::shared_ptr<Proc> proc) override
	{
		m_procs.push_back(proc);

		// total weight
		m_total_weight = 0;
		for(const auto& proc : m_procs)
			m_total_weight += proc->prio;
	}

	virtual const std::shared_ptr<Proc> Schedule() override
	{
		if(!m_procs.size())
			return nullptr;

		// sort by virtual time
		std::stable_sort(m_procs.begin(), m_procs.end(), [this](const auto& proc1, const auto& proc2) -> bool
		{
			// same virtual time => sort by weight
			if(std::abs(proc1->virtual_time-proc2->virtual_time) < m_eps)
				return proc1->prio >= proc2->prio;

			return proc1->virtual_time < proc2->virtual_time;
		});

		/*
		// alternative: total weight of remaining processes, not initial processes
		m_total_weight = 0;
		for(const auto& proc : m_procs)
			m_total_weight += proc->prio;
		*/

		auto proc = m_procs.front();

		proc->scheduled_time = m_total_weight / m_timeslice;
		if(proc->scheduled_time > proc->remaining_time)
			proc->scheduled_time = proc->remaining_time;
		proc->remaining_time -= proc->scheduled_time;
		proc->virtual_time += double(proc->scheduled_time) / double(proc->prio);

		if(proc->remaining_time <= 0)
			m_procs.pop_front();

		return proc;
	}

	virtual const char* GetSchedName() const override
	{
		return "PreemptCFS";
	}

private:
	std::deque<std::shared_ptr<Proc>> m_procs{};
	unsigned int m_timeslice{};

	unsigned int m_total_weight{};
	double m_eps{1e-6};
};


// ----------------------------------------------------------------------------


template<class t_scheds, std::size_t idx>
void tst_sched()
{
	using t_sched = typename std::tuple_element<idx, t_scheds>::type;
	ISched* sched = new t_sched();

	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=0, .remaining_time=10, .prio=1}));
	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=1, .remaining_time=20, .prio=3}));
	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=2, .remaining_time=30, .prio=2}));
	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=3, .remaining_time=10, .prio=3}));
	sched->AddProcess(std::make_shared<Proc>(Proc{.pid=4, .remaining_time=1, .prio=1}));


	std::cout << "Scheduler: " << sched->GetSchedName() << std::endl;
	while(1)
	{
		auto nextproc = sched->Schedule();
		if(!nextproc)
			break;

		std::cout.precision(4);
		std::cout << "Scheduling process " << nextproc->pid << " for " << nextproc->scheduled_time << " time units, "
			<< "remaining process time: " << nextproc->remaining_time
			<< ", virtual time: " << nextproc->virtual_time
			<< "." << std::endl;
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
	using t_scheds = std::tuple<CoopFCFS, CoopSJF, CoopPrio, PreemptRR, PreemptSRTF, PreemptPrio, PreemptCFS>;
	tst_sched<t_scheds>(std::make_index_sequence<std::tuple_size<t_scheds>::value>());

	return 0;
}
