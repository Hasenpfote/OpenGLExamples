/*!
* @file logger.h
* @brief Logger class.
* @author Hasenpfote
* @date 2016/07/03
*/
#pragma once
#include <string>
#include <memory>
#include <typeindex>

namespace hasenpfote{ namespace log{

class IAppender;

class Logger
{
public:
    enum class Severity
    {
        Verbose = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Fatal = 5
    };

public:
    Logger();
    virtual ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator = (const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator = (Logger&&) = delete;

    template<typename T>
    void AddAppender(const std::shared_ptr<IAppender>& appender)
    {
        static_assert(std::is_base_of<IAppender, T>::value == true, "T must be derived from IAppender.");
        AddAppender(typeid(T), appender);
    }

    template<typename T>
    void RemoveAppender()
    {
        static_assert(std::is_base_of<IAppender, T>::value == true, "T must be derived from IAppender.");
        RemoveAppender(typeid(T));
    }

    void SetSeverity(Severity severity);
    void Log(Severity severity, const std::string& filename, int line, const std::string& message);
    void SetTimestampFormat(const std::string& format); // Use for std::put_time.

private:
    void AddAppender(const std::type_index& index, const std::shared_ptr<IAppender>& appender);
    void RemoveAppender(const std::type_index& index);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

}}