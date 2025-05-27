#pragma once 

#include <vector>
#include <chrono>
#include <string>

namespace ecs
{
    namespace profiling
    {
        struct call_graph_t
        {
        public:
            struct node_t
            {
                node_t() = default;
                node_t(const std::string& inName, double inBeginTime, size_t inParentIndex)
                    : parentIndex{inParentIndex}, name{inName}, beginTime{inBeginTime}
                {}

                size_t parentIndex = 0;
                std::string name;
                double beginTime = 0.0;
                double endTime = 0.0;
                size_t depth = 0;

                inline double duration() const noexcept { return endTime - beginTime; }

                template<class Archive>
                void serialize(Archive& archive)
                {
                    archive(parentIndex, name, beginTime, endTime, depth);
                }
            };

            call_graph_t();

            void begin_call(const std::string& functionName, double beginTime);
            void end_call(const std::string& functionName, double endTime);
            void begin_frame(const double frameBeginTime);
            void end_frame(const double frameEndTime);

            template<class Archive>
            void serialize(Archive& archive)
            {
                archive(m_nodes);
            }

        private:
            std::vector<node_t> m_nodes;
            size_t m_lastAddedIndex = 0;
        };
    }
}