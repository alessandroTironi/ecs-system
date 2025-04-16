#include "Profiling/CycleCounter.h"
#include "Profiling/ProfilingSystem.h"

using namespace ecs;

CycleCounter::CycleCounter(const name& id)
{
	m_id = id;
	m_beginTime = std::chrono::high_resolution_clock::now();
}

CycleCounter::~CycleCounter()
{
	m_endTime = std::chrono::high_resolution_clock::now();

	SendToDataCollector();
}

void CycleCounter::SendToDataCollector()
{
	ProfilingDataCollector::instance()->AddCycleCounterData(*this);
}