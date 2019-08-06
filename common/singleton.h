/*!
* @file singleton.h
* @brief CRTP(Curiously Recurring Template Pattern) singleton class.
* @author Hasenpfote
* @date 2016/07/20
*/
#pragma once
#include <memory>
#include <functional>
#include <mutex>

namespace common
{

template<typename T>
class Singleton
{
protected:
    Singleton() = default;
    ~Singleton() = default;

public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    template<typename... Arguments>
    static T& GetMutableInstance(Arguments&&... args);

    template<typename... Arguments>
    static const T& GetConstInstance(Arguments&&... args);

private:
    template<typename... Arguments>
    static T& GetInstance(Arguments&&... args);

    static void DestroyInstance();

    static std::once_flag& GetOnceFlag();

    static std::unique_ptr<T> instance;
};

class SingletonFinalizer final
{
    template<typename T>
    friend class Singleton;

public:
    using Callback = std::function<void()>;

public:
    SingletonFinalizer() = delete;
    ~SingletonFinalizer() = delete;
    SingletonFinalizer(const SingletonFinalizer&) = delete;
    SingletonFinalizer& operator=(const SingletonFinalizer&) = delete;
    SingletonFinalizer(SingletonFinalizer&&) = delete;
    SingletonFinalizer& operator=(SingletonFinalizer&&) = delete;

private:
    static void Register(const Callback& cb);
    static void Finalize();
};

}   // namespace common

#include "impl/singleton_impl.h"