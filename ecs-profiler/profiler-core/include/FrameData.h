#pragma once

#include <unordered_map>
#include <chrono>
#include <limits>
#include <string>
#include "CycleCounter.h"

#include <cereal/types/chrono.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>

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
			size_t depth = 0;

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(totalTimeMs, maxTimeMs, minTimeMs, averageTimeMs, framePercent, depth);
			}
		};

		struct frame_data_t
		{
			std::unordered_map<cycle_counter_name, cycle_counter_data_t> countersData;
			double frameBeginTime;
			double frameEndTime;

			void aggregate_data();
			std::string to_string() const;

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(countersData, frameBeginTime, frameEndTime);
			}
		};
	}
}