#include "FileWriter.h"
#include "Session.h"
#include <fstream>
#include <iostream>



using namespace ecs::profiling;

FileWriter::FileWriter(std::string fileName)
	: m_fileName{fileName}
{

}

void FileWriter::Write(std::shared_ptr<Session> session)
{
	if (session == nullptr)
	{
		return;
	}

	m_session = session;

	m_isWritingToFile.store(true, std::memory_order_relaxed);

	m_writingThread = std::thread(&FileWriter::WriteInternal, this);
}

void FileWriter::JoinWritingThread()
{
	if (m_writingThread.joinable())
	{
		m_writingThread.join();
	}
}

void FileWriter::WriteInternal()
{
	std::ofstream file(m_fileName, std::ios::binary);
	if (!file)
	{
		std::cerr << "Failed to open file for writing" << std::endl;
		m_isWritingToFile.store(false, std::memory_order_relaxed);
		return;
	}

	cereal::BinaryOutputArchive archive(file);
	archive(*m_session);

	m_isWritingToFile.store(false, std::memory_order_relaxed);
}