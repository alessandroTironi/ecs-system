#pragma once

#include <string>
#include <cstring>
#include <functional>

namespace ecs 
{
    template<size_t N>
    class compact_string 
    {
    public:
        compact_string() : m_size(0) 
        {
            m_data[0] = '\0';
        }

        compact_string(const std::string& str) : compact_string(str.c_str()) 
        {}

        compact_string(const char* str) 
        {
            size_t len = std::strlen(str);
            if (len >= N) 
            {
                len = N - 1;
            }
            strncpy_s(m_data, N, str, len);
            m_data[len] = '\0';
            m_size = len;
        }

        compact_string(const compact_string& other) 
        {
            strncpy_s(m_data, N, other.m_data, N);
            m_size = other.m_size;
        }

        compact_string& operator=(const compact_string& other) 
        {
            if (this != &other) 
            {
                strncpy_s(m_data, N, other.m_data, N);
                m_size = other.m_size;
            }
            return *this;
        }

        compact_string& operator=(const std::string& str) 
        {
            return *this = str.c_str();
        }

        compact_string& operator=(const char* str) 
        {
            size_t len = std::strlen(str);
            if (len >= N) 
            {
                len = N - 1;
            }
            strncpy_s(m_data, N, str, len);
            m_data[len] = '\0';
            m_size = len;
            return *this;
        }

        bool operator==(const compact_string& other) const 
        {
            return std::strcmp(m_data, other.m_data) == 0;
        }

        bool operator!=(const compact_string& other) const 
        {
            return !(*this == other);
        }

        bool operator==(const std::string& str) const 
        {
            return std::strcmp(m_data, str.c_str()) == 0;
        }

        bool operator==(const char* str) const 
        {
            return std::strcmp(m_data, str) == 0;
        }


        inline size_t size() const { return m_size; }
        
        inline size_t capacity() const { return N; }
        
        inline const char* c_str() const { return m_data; }

    private:
        char m_data[N];
        size_t m_size;
    };

}


namespace std 
{
    template<size_t N>
    struct hash<ecs::compact_string<N>> 
    {
        size_t operator()(const ecs::compact_string<N>& str) const 
        {
            // FNV-1a hash
            size_t hash = 14695981039346656037ULL; // FNV offset basis
            const char* data = str.c_str();
            for (size_t i = 0; i < str.size(); ++i)
            {
                hash ^= static_cast<size_t>(data[i]);
                hash *= 1099511628211ULL; // FNV prime
            }
            return hash;
        }
    };
}