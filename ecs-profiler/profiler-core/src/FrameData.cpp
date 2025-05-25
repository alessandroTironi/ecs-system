#include "FrameData.h"
#include <format>
#include <string>

using namespace ecs::profiling;

void frame_data_t::aggregate_data()
{
	const double frameTime = std::chrono::duration<double, std::milli>(frameEndTime - frameBeginTime).count();

	double totalTime = 0.0;
	for (auto it = countersData.begin(); it != countersData.end(); ++it)
	{
		cycle_counter_data_t& data = it->second;
		data.framePercent = (data.totalTimeMs / frameTime) * 100.0;
		totalTime += data.totalTimeMs;

		//ECS_LOG(Log, "{}: \ttime: {:.4f}ms ({:.2f}%)", it->first, data.totalTimeMs, data.framePercent);
	}

	//ECS_LOG(Log, "Other: \t\ttime: {:.4f}ms ({:.2f}%)", frameTime - totalTime, 
	//		(1.0 - (totalTime / frameTime)) * 100.0);
}

std::string frame_data_t::to_string() const
{
	const double frameTime = std::chrono::duration<double, std::milli>(frameEndTime - frameBeginTime).count();
	std::string str = std::format("Duration: {:.4f}ms", frameTime);
	double totalTime = 0.0;
	
	for (auto it = countersData.begin(); it != countersData.end(); ++it)
	{
		cycle_counter_data_t data = it->second;
		data.framePercent = (data.totalTimeMs / frameTime) * 100.0;
		totalTime += data.totalTimeMs;

		str += std::format("\n\t{}: \ttime: {:.4f}ms ({:.2f}%)", it->first, data.totalTimeMs, data.framePercent);
	}

	str += std::format("\n\tOther: \ttime: {:.4f}ms ({:.2f}%)", frameTime - totalTime,
		(1.0 - (totalTime / frameTime)) * 100.0);

	return str;
}