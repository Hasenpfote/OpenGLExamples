#pragma once
#include <sstream>
#include <iostream>
#include <hasenpfote/log/logger.h>
#include "singleton.h"

class Logger final : public hasenpfote::log::Logger, public Singleton<Logger>
{
    friend class Singleton<Logger>;
private:
    Logger() = default;
public:
    ~Logger() = default;
};

// Convenience macro's.

#define SHORT_FILENAME (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : std::strrchr(__FILE__, '\\') ? std::strrchr(__FILE__, '\\') + 1 : __FILE__)

#define LOG(severity, message)\
    do{\
        std::ostringstream oss;\
        oss << message;\
        ::Logger::GetMutableInstance().Log(severity, SHORT_FILENAME, __LINE__, oss.str());\
    }while (false)

#define NOLOG(message)\
    do{\
        static_cast<void>((true? static_cast<void>(0) : static_cast<void>((std::ostringstream() << message))));\
    }while(false)

#ifdef NDEBUG
#define LOG_V(message) NOLOG(message)
#define LOG_D(message) NOLOG(message)
#else
#define LOG_V(message) LOG(::Logger::Severity::Verbose, message)
#define LOG_D(message) LOG(::Logger::Severity::Debug, message)
#endif
#define LOG_I(message) LOG(::Logger::Severity::Info, message)
#define LOG_W(message) LOG(::Logger::Severity::Warning, message)
#define LOG_E(message) LOG(::Logger::Severity::Error, message)
#define LOG_F(message) LOG(::Logger::Severity::Fatal, message)