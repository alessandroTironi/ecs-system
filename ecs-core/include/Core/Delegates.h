#pragma once 

#include <functional>


namespace ecs 
{
    template<typename TReturnType, typename... TArgs>
    struct delegate_t
    {
        using FunctionType = std::function<TReturnType(TArgs...)>;

        delegate_t() = default;

        TReturnType operator()(TArgs... args)
        {
            return m_function(args...);
        }

        static delegate_t<TReturnType, TArgs...> create(FunctionType function)
        {
            return delegate_t<TReturnType, TArgs...>(function);
        }

    private:
        delegate_t(FunctionType function) : m_function{function} {}

        FunctionType m_function;
    };
}