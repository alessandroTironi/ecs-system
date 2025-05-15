#pragma once 

#include <functional>


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
            return delegate_t<TReturnType, TArgs...>(std::move(function));
        }

    private:
        delegate_t(FunctionType function) : m_function{function} {}

        FunctionType m_function = nullptr;
    };
}

#define DEFINE_VOID_DELEGATE(DelegateTypeName, ...) using DelegateTypeName = ecs::delegate_t<void, ## __VA_ARGS__>

#define DEFINE_DELEGATE_RET(DelegateTypeName, ReturnType, ...) using DelegateTypeName = ecs::delegate_t<ReturnType, ## __VA_ARGS__>