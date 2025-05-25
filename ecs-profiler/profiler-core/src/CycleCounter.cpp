#include "CycleCounter.h"
#include "Profiler.h"
#include <stacktrace>

using namespace ecs::profiling;

ScopeCycleCounter::ScopeCycleCounter(cycle_counter_name id)
{
	if (id.size() > 0)
	{
		m_valid = true;

		m_recordBeginTime = std::chrono::high_resolution_clock::now();
		m_id = id;
	}
}

ScopeCycleCounter::~ScopeCycleCounter()
{
	if (m_id.size() > 0 && m_valid)
	{
		m_recordEndTime = std::chrono::high_resolution_clock::now();

		Profiler::Instance()->AddCycleCounter(*this);
	}
}