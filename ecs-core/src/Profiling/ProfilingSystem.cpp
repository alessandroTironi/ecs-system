#include "Profiling/ProfilingSystem.h"
#include "Containers/memory.h"

#include <iostream>
#include <unordered_map>

using namespace ecs;

ProfilingDataCollector::ProfilingDataCollector()
{
	m_cycleCounters.reserve(1024);
}

ProfilingDataCollector::~ProfilingDataCollector()
{
	
}

void ProfilingDataCollector::StartNewFrame()
{
	m_lastFrameBeginTime = std::chrono::high_resolution_clock::now();
}

void ProfilingDataCollector::EndFrame()
{
	const std::chrono::high_resolution_clock::time_point frameEndTime = std::chrono::high_resolution_clock::now();
	const std::chrono::high_resolution_clock::duration frameTime = frameEndTime - m_lastFrameBeginTime;

	std::unordered_map<name, std::chrono::milliseconds> cycleCounterMap;
	for (size_t i = 0; i < m_cycleCounters.size(); ++i)
	{
		cycle_data_t& data = m_cycleCounters[i];
		auto optionalData = cycleCounterMap.find(data.id);
		if (optionalData != cycleCounterMap.end())
		{
			optionalData->second += data.duration;
		}
		else
		{
			cycleCounterMap[data.id] = data.duration;
		}
	}
	m_cycleCounters.clear();

	std::chrono::milliseconds trackedTotal = std::chrono::milliseconds();
	for (auto cycleIt = cycleCounterMap.begin(); cycleIt != cycleCounterMap.end(); ++cycleIt)
	{
		std::cout << cycleIt->first.c_str() << ": " << cycleIt->second.count() / 1000000.0 << " ms" << std::endl;
		trackedTotal += cycleIt->second;
		cycleIt->second = std::chrono::milliseconds();
	}

	std::cout << "Other: " << (frameTime - trackedTotal).count() / 1000000.0 << " ms" << std::endl;
}

void ProfilingDataCollector::AddCycleCounterData(const CycleCounter& counter)
{
	m_cycleCounters.emplace_back(counter.id(), counter.duration());
}