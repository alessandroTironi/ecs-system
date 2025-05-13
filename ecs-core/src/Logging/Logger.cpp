#include "Logging/Logger.h"
#include <iostream>
#include <windows.h>

using namespace ecs;

Logger::Logger(bool autoRun)
{
	m_buffer.resize(capacity);

	if (autoRun)
	{
		Run();
	}
}

Logger::~Logger()
{
	Stop();
}

void Logger::Run()
{
	m_consumerThread = std::thread(&Logger::ConsumeBuffer, this);
}

void Logger::Stop()
{
	m_running.store(false, std::memory_order_relaxed);

	if (m_consumerThread.joinable())
	{
		m_consumerThread.join();
	}
}

void Logger::Log(const std::string& message, ELogVerbosity verbosity)
{
	if (m_bufferCount.load(std::memory_order_relaxed) >= m_buffer.size())
	{
		return; // Buffer is full, drop the message
	}

	m_buffer[m_producerIndex.load(std::memory_order_relaxed)] = log_message_t(message, verbosity);
	m_producerIndex.store((m_producerIndex.load(std::memory_order_relaxed) + 1) % m_buffer.size(), 	
		std::memory_order_relaxed);
	m_bufferCount.fetch_add(1, std::memory_order_relaxed);
}

void Logger::ConsumeBuffer()
{
	m_running.store(true, std::memory_order_relaxed);

	while (m_running.load(std::memory_order_relaxed))
	{
		if (m_bufferCount.load(std::memory_order_relaxed) > 0)
		{
			const log_message_t& message = m_buffer[m_consumerIndex.load(std::memory_order_relaxed)];
			m_consumerIndex.store((m_consumerIndex.load(std::memory_order_relaxed) + 1) % m_buffer.size(), 
				std::memory_order_relaxed);

			m_bufferCount.fetch_sub(1, std::memory_order_relaxed);

			int textColor = 0x07; // Default white text
			switch (message.verbosity)
			{
				case ELogVerbosity::VeryVerbose: textColor = 0x0B; break;
				case ELogVerbosity::Verbose: textColor = 0x0B; break; 
				case ELogVerbosity::Log: textColor = 0x07; break;
				case ELogVerbosity::Warning: textColor = 0x0E; break; 
				case ELogVerbosity::Error: textColor = 0x0C; break; 
				case ELogVerbosity::Fatal: textColor = 0x0D; break; 
			}

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), textColor);
			std::cout << "[" << message.timestamp << "]: " << message.message << std::endl;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07); // Reset to default text color
		}	
	}
}