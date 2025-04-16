#pragma once 

#include <chrono>
#include <vector>
#include "Core/Types.h"
#include "CycleCounter.h"

namespace ecs
{
	struct frame_data_t 
	{
		real_t beginTime = 0.0f;
		real_t endTime = 0.0f;

	};

	class ProfilingDataCollector
	{
	public:
		ProfilingDataCollector();
		~ProfilingDataCollector();

		void StartNewFrame();
		void EndFrame();

		void AddCycleCounterData(const CycleCounter& counter);

		static inline ProfilingDataCollector* instance() 
		{
			if (!m_instance.get())
			{
				m_instance = std::make_shared<ProfilingDataCollector>();
			}

			return m_instance.get();
		}

	private:
		struct cycle_data_t
		{
			cycle_data_t(const name& inId, const std::chrono::milliseconds inDuration)
				: id{inId}, duration{inDuration} {}

			name id;
			std::chrono::milliseconds duration;
		};

		/** A plain array containing data from the registered cycle counters. */
		std::vector<cycle_data_t> m_cycleCounters;

		std::chrono::high_resolution_clock::time_point m_lastFrameBeginTime;

		static std::shared_ptr<ProfilingDataCollector> m_instance;
	};
}