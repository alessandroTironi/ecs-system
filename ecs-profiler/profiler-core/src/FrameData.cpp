#include "FrameData.h"
#include <format>
#include <string>

using namespace ecs::profiling;

frame_data_t::frame_data_t()
{
	
}

void frame_data_t::register_frame_begin(double beginTime)
{
	callGraph.begin_frame(beginTime);
}

void frame_data_t::register_frame_end(double endTime)
{
	callGraph.end_frame(endTime);
}