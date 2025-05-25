#include "Session.h"
#include <fstream>
#include <iostream>

#include <cereal/archives/binary.hpp>

using namespace ecs::profiling;

void Session::PushFrameData(const frame_data_t& frameData)
{
	m_frameData.push_back(frameData);
}

void Session::Clear()
{
	m_frameData.clear();
}

Session* Session::CreateFromFile(const std::string& fileName)
{
	Session* session = new Session();
	session->ReadFromFile(fileName);

	return session;
}

void Session::ReadFromFile(const std::string& fileName)
{
	std::ifstream file(fileName, std::ios::binary);
	if (!file)
	{
		std::cerr << "Failed to open file " << fileName << std::endl;
		return;
	}

	cereal::BinaryInputArchive iarchive(file);
	iarchive(*this);

	file.close();
}

