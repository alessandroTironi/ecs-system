#pragma once 

#include <limits>
#include <stdexcept>

namespace ecs
{
    /*  This class can be used to generate unique IDs.
        Each call to GenerateNewUniqueID increments a counter that ensures the
        next returned ID will be unique and never assigned before. */
    template<typename IDType>
    class IDGenerator
    {
    public:
        IDGenerator()
        {
            m_nextID = 0;
            m_maxID = std::numeric_limits<IDType>::max();
        }

        IDType GenerateNewUniqueID() 
        {
            if (m_nextID == m_maxID)
            {
                throw std::overflow_error("Reached maximum counter of unique ID");
            }

            return m_nextID++;
        }

        void Reset()
        {
            m_nextID = 0;
        }

    private:
        IDType m_nextID = 0;
        IDType m_maxID = 0;
    };
}
