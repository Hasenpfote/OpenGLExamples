#pragma once
#include <cassert>

template<typename T> std::unique_ptr<T> Singleton<T>::instance = nullptr;

template<typename T>
template<typename... Arguments>
T& Singleton<T>::GetMutableInstance(Arguments&&... args)
{
    return GetInstance(std::forward<Arguments>(args)...);
}

template<typename T>
template<typename... Arguments>
const T& Singleton<T>::GetConstInstance(Arguments&&... args)
{
    return GetInstance(std::forward<Arguments>(args)...);
}

template<typename T>
template<typename... Arguments>
T& Singleton<T>::GetInstance(Arguments&&... args)
{
    std::call_once(
        GetOnceFlag(),
        [](Arguments&&... args)
        {
            instance.reset(new T(std::forward<Arguments>(args)...));
            SingletonFinalizer::Register(std::bind(&Singleton<T>::DestroyInstance));
        },
        std::forward<Arguments>(args)...
        );
    assert(instance);
    return *instance;
}

template<typename T>
void Singleton<T>::DestroyInstance()
{
    if(instance)
        instance.reset();
}

template<typename T>
std::once_flag& Singleton<T>::GetOnceFlag()
{
    static std::once_flag once;
    return once;
}