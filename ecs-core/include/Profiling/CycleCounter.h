#pragma once 

#include <chrono>
#include "Core/Types.h"

namespace ecs
{
	class CycleCounter 
	{
	public:
		CycleCounter() = default;
		CycleCounter(const name& id);
		~CycleCounter();

		inline const name& id() const { return m_id; }
		inline const std::chrono::milliseconds duration() const 
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(m_endTime - m_beginTime); 
		}

	private:
		void SendToDataCollector();

		name m_id;
		std::chrono::high_resolution_clock::time_point m_beginTime;
		std::chrono::high_resolution_clock::time_point m_endTime;
	};
}