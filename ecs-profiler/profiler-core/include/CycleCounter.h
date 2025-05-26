#pragma once 

#include <chrono>

namespace ecs
{
	namespace profiling
	{
		using cycle_counter_name = std::string;

		class ScopeCycleCounter
		{
		public:
			ScopeCycleCounter() = default;
			ScopeCycleCounter(cycle_counter_name id);
			~ScopeCycleCounter();

			inline const cycle_counter_name& id() const noexcept { return m_id; }
			inline double duration_ms() const noexcept 
			{
				return std::chrono::duration<double, std::milli>(m_recordEndTime - m_recordBeginTime).count();
			}

			inline void invalidate() noexcept { m_valid = false; }
			inline size_t depth() const noexcept { return m_depth; }
		private:
			std::chrono::high_resolution_clock::time_point m_recordBeginTime;
			std::chrono::high_resolution_clock::time_point m_recordEndTime;
			size_t m_depth = 0;
			cycle_counter_name m_id;
			bool m_valid = false;

			static uint32_t s_currrentDepth;
		};
	}
}

#ifdef DEBUG_BUILD
#define SCOPE_CYCLE_COUNTER(Id) ecs::profiling::ScopeCycleCounter __ScopeCycleCounter_##Id(#Id)
#else 
#define SCOPE_CYCLE_COUNTER(Id)
#endif