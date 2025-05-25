#pragma once 

#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include "Profiler.h"
#include "FrameData.h"

#include <cereal/archives/binary.hpp>

namespace ecs
{
	namespace profiling
	{
		class Session;

		class FileWriter
		{
		public:
			FileWriter() = default;
			FileWriter(std::string fileName);

			void Write(std::shared_ptr<Session> session);
			void JoinWritingThread();

		private:
			void WriteInternal();

			std::thread m_writingThread;
			std::string m_fileName;
			std::atomic<bool> m_isWritingToFile{false};

			std::shared_ptr<Session> m_session;
		};
	}
}