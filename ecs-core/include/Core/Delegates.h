#pragma once 

#include <functional>
#include <vector>
#include <algorithm>
#include "Core/IDGenerator.h"

namespace ecs 
{
    template<typename TReturnType, typename... TArgs>
    struct delegate_t
    {
        using FunctionType = std::function<TReturnType(TArgs...)>;

        delegate_t() = default;

        TReturnType invoke(TArgs... args)
        {
            if constexpr (std::is_same_v<TReturnType, void>)
            {
                m_function(args...);
            }
            else
            {
                return m_function(args...);
            }
        }

        bool is_bound() const noexcept
        {
            return m_function != nullptr;
        }

        static delegate_t<TReturnType, TArgs...> create(FunctionType&& function)
        {
            return delegate_t<TReturnType, TArgs...>(std::move(function),
                s_idGenerator.GenerateNewUniqueID());
        }

        bool operator==(const delegate_t<TReturnType, TArgs...>& other) const noexcept
        {
            return m_uniqueID == other.m_uniqueID;
        }

        inline size_t unique_id() const noexcept { return m_uniqueID; }

    private:
        delegate_t(FunctionType function, const size_t inID) 
            : m_function{function}, m_uniqueID{inID} {}

        FunctionType m_function = nullptr;
        size_t m_uniqueID;

        inline static IDGenerator<size_t> s_idGenerator;
    };

    template<typename... TArgs>
    struct mc_delegate_t
    {
        using FunctionType = std::function<void(TArgs...)>;
        using DelegateType = delegate_t<void, TArgs...>;

        mc_delegate_t() = default;

        void broadcast(TArgs... args)
        {
            for (auto& delegate : m_delegates)
            {
                delegate.invoke(args...);
            }
        }

        void operator+=(DelegateType delegate)
        {
            m_delegates.push_back(delegate);
        }

        void operator-=(DelegateType delegate)
        {
            m_delegates.erase(std::remove(m_delegates.begin(), m_delegates.end(), delegate), m_delegates.end());
        }

        static DelegateType create(FunctionType&& function)
        {
            return DelegateType::create(std::move(function));
        }

    private:
        std::vector<DelegateType> m_delegates;
    };
}

#define DEFINE_VOID_DELEGATE(DelegateTypeName, ...) using DelegateTypeName = ecs::delegate_t<void, ## __VA_ARGS__>

#define DEFINE_DELEGATE_RET(DelegateTypeName, ReturnType, ...) using DelegateTypeName = ecs::delegate_t<ReturnType, ## __VA_ARGS__>

#define DEFINE_MULTICAST_DELEGATE(DelegateTypeName, ...) using DelegateTypeName = ecs::mc_delegate_t< __VA_ARGS__>