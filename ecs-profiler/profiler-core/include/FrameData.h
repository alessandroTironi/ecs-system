#pragma once

#include <unordered_map>
#include <chrono>
#include <limits>
#include "CycleCounter.h"

namespace ecs
{
	namespace profiling
	{
		struct cycle_counter_data_t
		{
			double totalTimeMs = 0.0;
			double maxTimeMs = 0.0;
			double minTimeMs = std::numeric_limits<double>::max();
			double averageTimeMs = 0.0;
			double framePercent = 0.0;
		};

		struct frame_data_t
		{
			std::unordered_map<cycle_counter_name, cycle_counter_data_t> countersData;
			std::chrono::high_resolution_clock::time_point frameBeginTime;
			std::chrono::high_resolution_clock::time_point frameEndTime;

			void aggregate_data();
		};
	}
}