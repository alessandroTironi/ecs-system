#include "CallGraph.h"

#include <cassert>

using namespace ecs::profiling;

call_graph_t::call_graph_t()
{
    m_nodes.reserve(8);
}

void call_graph_t::begin_frame(const double frameBeginTime)
{
    assert(m_nodes.size() == 0);

    node_t firstNode;
    firstNode.name = "Frame";
    firstNode.parentIndex = 0;
    firstNode.beginTime = frameBeginTime;

    m_nodes.push_back(firstNode);
    m_lastAddedIndex = 0;
}

void call_graph_t::end_frame(const double frameEndTime)
{
    m_nodes[0].endTime = frameEndTime;
}

void call_graph_t::begin_call(const std::string& functionName, double beginTime)
{
    m_nodes.emplace_back(functionName, beginTime, m_lastAddedIndex);
    m_nodes[m_nodes.size() - 1].depth = m_nodes[m_lastAddedIndex].depth + 1;
    m_lastAddedIndex = m_nodes.size() - 1;
}

void call_graph_t::end_call(const std::string& functionName, double endTime)
{
    m_nodes[m_lastAddedIndex].endTime = endTime;

    m_lastAddedIndex = m_nodes[m_lastAddedIndex].parentIndex;
}