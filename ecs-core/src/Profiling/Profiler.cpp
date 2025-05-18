#include "Profiling/Profiler.h"

using namespace ecs::profiling;

std::unique_ptr<Profiler> Profiler::m_instance = std::make_unique<Profiler>();

Profiler::Profiler()
{
	m_frameBuffer.resize_buffer(128);
}

Profiler::~Profiler()
{
	StopRecording();
}

void Profiler::StartRecording()
{
	if (!m_running.load(std::memory_order_relaxed))
	{
		m_running.store(true, std::memory_order_relaxed);
		m_profilerThread = std::thread(&Profiler::ProcessData, this);
	}
	
}

void Profiler::StopRecording()
{
	if (m_running.load(std::memory_order_relaxed))
	{
		m_running.store(false, std::memory_order_relaxed);

		if (m_profilerThread.joinable())
		{
			m_profilerThread.join();
		}
	}
}

bool Profiler::IsRecording() const noexcept 
{
	return m_running.load(std::memory_order_relaxed);
}

void Profiler::ProcessData()
{
	while (m_running.load(std::memory_order_relaxed))
	{
		ScopeCycleCounter counter;
		if (m_endFrameProcessing.load(std::memory_order_relaxed))
		{
			while (m_frameBuffer.consume_item(counter))
			{
				ProcessCycleCounter(counter);
			}

			m_currentFrameData.frameEndTime = std::chrono::high_resolution_clock::now();
			m_currentFrameData.aggregate_data();
			m_recordedFrames.push_back(m_currentFrameData);

			m_currentFrameData.frameBeginTime = std::chrono::high_resolution_clock::now();
			m_currentFrameData.countersData.clear();

			m_endFrameProcessing.store(false, std::memory_order_relaxed);
		}

		if (!m_frameBuffer.consume_item(counter))
		{
			continue;
		}

		ProcessCycleCounter(counter);
		counter.invalidate();
	}

	m_currentFrameData.aggregate_data();
	m_recordedFrames.push_back(m_currentFrameData);
	m_recordedFrames.clear();
}

void Profiler::ProcessCycleCounter(const ScopeCycleCounter& counter)
{
	cycle_counter_data_t& counterData = m_currentFrameData.countersData[counter.id()];
	const double duration = counter.duration_ms();
	counterData.totalTimeMs += duration;
	if (duration > counterData.maxTimeMs)
	{
		counterData.maxTimeMs = duration;
	}

	if (duration < counterData.minTimeMs)
	{
		counterData.minTimeMs = duration;
	}
}

void Profiler::AddCycleCounter(const ScopeCycleCounter& counter)
{
	if (m_running.load(std::memory_order_relaxed))
	{
		m_frameBuffer.produce_item(counter);
	}
}

void Profiler::StartNewFrame()
{
	m_endFrameProcessing.store(true, std::memory_order_relaxed);
}