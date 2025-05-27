#pragma once

#include <unordered_map>
#include <chrono>
#include <limits>
#include <string>
#include "CycleCounter.h"
#include "CallGraph.h"

#include <cereal/types/chrono.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>

namespace ecs
{
	namespace profiling
	{
		

		struct frame_data_t
		{
			frame_data_t();

			call_graph_t callGraph;

			void register_frame_begin(double beginTime);
			void register_frame_end(double endTime);

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(callGraph);
			}
		};
	}
}