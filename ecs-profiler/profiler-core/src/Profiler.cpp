#include "Profiler.h"
#include "FileWriter.h"
#include "Session.h"

using namespace ecs::profiling;

std::unique_ptr<Profiler> Profiler::m_instance = std::make_unique<Profiler>();

Profiler::Profiler()
{
	m_cycleCountersBuffer.reserve(m_capacity);
	m_cycleCountersBuffer.resize(m_capacity);

	m_session = std::make_shared<Session>();
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
			while (TryPopCounter(counter))
			{
				ProcessCycleCounter(counter);
			}

			m_currentFrameData.frameEndTime = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			m_currentFrameData.aggregate_data();
			m_session->PushFrameData(m_currentFrameData);

			m_currentFrameData.frameBeginTime = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			m_currentFrameData.countersData.clear();

			m_endFrameProcessing.store(false, std::memory_order_relaxed);
		}

		if (!TryPopCounter(counter))
		{
			continue;
		}

		ProcessCycleCounter(counter);
		counter.invalidate();
	}

	m_currentFrameData.aggregate_data();
	m_session->PushFrameData(m_currentFrameData);

	std::unique_ptr<FileWriter> writer = std::make_unique<FileWriter>("profiler-session.bin");
	if (writer != nullptr)
	{
		writer->Write(m_session);
		writer->JoinWritingThread();
	}

	m_session->Clear();
}

void Profiler::ProcessCycleCounter(const ScopeCycleCounter& counter)
{
	cycle_counter_data_t& counterData = m_currentFrameData.countersData[counter.id()];
	const double duration = counter.duration_ms();
	counterData.totalTimeMs += duration;
	counterData.depth = counter.depth();
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
		TryPushCounter(counter);
	}
}

void Profiler::StartNewFrame()
{
	m_endFrameProcessing.store(true, std::memory_order_relaxed);
}

bool Profiler::TryPopCounter(ScopeCycleCounter& outCounter)
{
	if (m_bufferCount.load(std::memory_order_acquire) == 0)
	{
		return false;
	}

	const size_t index = m_consumerIndex.load(std::memory_order_relaxed);
	outCounter = m_cycleCountersBuffer[index]; 
	
	// Use release ordering to ensure index update is visible to producers
	m_consumerIndex.store((index + 1) % m_capacity, std::memory_order_release);
	m_bufferCount.fetch_sub(1, std::memory_order_release);
	return true;
}

bool Profiler::TryPushCounter(const ScopeCycleCounter& counter)
{
	if (m_bufferCount.load(std::memory_order_acquire) >= m_capacity)
	{
		return false;
	}

	const size_t index = m_producerIndex.load(std::memory_order_relaxed);
	m_cycleCountersBuffer[index] = counter; 
	
	// Use release ordering to ensure data is visible before index update
	m_producerIndex.store((index + 1) % m_capacity, std::memory_order_release);
	m_bufferCount.fetch_add(1, std::memory_order_release);
	return true;
}