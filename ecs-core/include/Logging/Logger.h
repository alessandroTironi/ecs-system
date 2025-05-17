#pragma once 

#include <memory>
#include <string>
#include <atomic>
#include <vector>
#include <thread>
#include <chrono>
#include <format>

#include "Containers/RingBuffer.h"

namespace ecs
{
	enum class ELogVerbosity : uint8_t
	{
		VeryVerbose,
		Verbose,
		Log,
		Warning,
		Error,
		Fatal
	};

	class Logger 
	{
	public:
		Logger(bool autoRun = false);
		~Logger();

		void Run();
		void ConsumeBuffer();
		void Stop();

		void Log(const std::string& message, ELogVerbosity verbosity);

		template<typename... Args>
		void Log(ELogVerbosity verbosity, const std::string& message, Args&&... args)
		{
			const std::string formattedMessage = std::vformat(message, std::make_format_args(args...));
			Log(formattedMessage, verbosity);
		}

	private:
		struct log_message_t
		{
			std::string message;
			std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
			ELogVerbosity verbosity;

			log_message_t() = default;
			log_message_t(const std::string& msg, ELogVerbosity verb)
				: message{msg}, timestamp{std::chrono::high_resolution_clock::now()}, verbosity{verb} 
			{}
		};

		ring_buffer<log_message_t> m_buffer; 
		std::thread m_consumerThread; 
		std::atomic<bool> m_running{false};
	};

	inline static Logger* gLogger = new Logger(true);
}

#if DEBUG_BUILD
#define ECS_LOG(verbosity, message, ...) ecs::gLogger->Log(ecs::ELogVerbosity::verbosity, \
			message __VA_OPT__(,) __VA_ARGS__);
#else
#define ECS_LOG(verbosity, message) 
#endif