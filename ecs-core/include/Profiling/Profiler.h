#pragma once

#include <atomic>
#include <thread>
#include <unordered_map>
#include <vector>
#include "Containers/RingBuffer.h"
#include "Core/Types.h"
#include "CycleCounter.h"
#include "FrameData.h"

#define ENABLE_PROFILER DEBUG_BUILD


namespace ecs
{
	namespace profiling
	{
		namespace gui 
		{
			class ProfilerGraphicsApp;
		}

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

			std::thread m_profilerThread;
			std::atomic<bool> m_running = false;
			ring_buffer<ScopeCycleCounter> m_frameBuffer;

			frame_data_t m_currentFrameData;
			std::vector<frame_data_t> m_recordedFrames;
			std::atomic<bool> m_endFrameProcessing{false};

			std::unique_ptr<gui::ProfilerGraphicsApp> m_gui;

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