#pragma once

#include <atomic>
#include <thread>
#include <unordered_map>
#include <vector>
#include "CycleCounter.h"
#include "FrameData.h"

#define ENABLE_PROFILER DEBUG_BUILD


namespace ecs
{
	namespace profiling
	{
		class Profiler
		{
		public:
			Profiler();
			~Profiler();

			inline static Profiler* Instance() noexcept { return m_instance.get(); }

			void StartRecording();
			void StopRecording();
			bool IsRecording() const noexcept;

			void AddCycleCounter(const ScopeCycleCounter& counter);
			void StartNewFrame();

		private:
			void ProcessData();
			void ProcessCycleCounter(const ScopeCycleCounter& counter);

			bool TryPopCounter(ScopeCycleCounter& outCounter);
			bool TryPushCounter(const ScopeCycleCounter& counter);

			std::thread m_profilerThread;
			std::atomic<bool> m_running = false;
			std::vector<ScopeCycleCounter> m_cycleCountersBuffer;
			std::atomic<size_t> m_bufferCount{0};
			std::atomic<size_t> m_producerIndex{0};
			std::atomic<size_t> m_consumerIndex{0};
			size_t m_capacity = 128;

			frame_data_t m_currentFrameData;
			std::vector<frame_data_t> m_recordedFrames;
			std::atomic<bool> m_endFrameProcessing{false};

			static std::unique_ptr<Profiler> m_instance;
		};
	}
}

#ifdef ENABLE_PROFILER

#define PROFILER_RUN()  ecs::profiling::Profiler::Instance()->StartRecording(); 
#define PROFILER_START_NEW_FRAME() ecs::profiling::Profiler::Instance()->StartNewFrame(); 
#define PROFILER_STOP() ecs::profiling::Profiler::Instance()->StopRecording(); 

#else 

#define PROFILER_RUN()
#define PROFILER_START_NEW_FRAME() 
#define PROFILER_STOP() 

#endif