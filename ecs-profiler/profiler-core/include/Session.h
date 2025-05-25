#pragma once

#include <vector>
#include <string>
#include "FrameData.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

namespace ecs
{
	namespace profiling
	{
		class Session
		{
		public:
			Session() = default;

			void PushFrameData(const frame_data_t& frameData);
			void Clear();

			static Session* CreateFromFile(const std::string& fileName);

			inline const std::vector<frame_data_t>& GetFrameData() const { return m_frameData; }

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(m_frameData);
			}

		private:
			void ReadFromFile(const std::string& fileName);

			std::vector<frame_data_t> m_frameData;
		};
	}
}