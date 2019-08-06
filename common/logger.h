#pragma once
#include <sstream>
#include <iostream>
#include <hasenpfote/log/logger.h>
#include "singleton.h"

namespace common
{

class Logger final : public hasenpfote::log::Logger, public common::Singleton<Logger>
{
    friend class common::Singleton<Logger>;
private:
    Logger() = default;
public:
    ~Logger() = default;
};

}   // namespace common

// Convenience macro's.

#define SHORT_FILENAME (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : std::strrchr(__FILE__, '\\') ? std::strrchr(__FILE__, '\\') + 1 : __FILE__)

#define LOG(severity, message)\
    do{\
        std::ostringstream oss;\
        oss << message;\
        common::Logger::GetMutableInstance().Log(severity, SHORT_FILENAME, __LINE__, oss.str());\
    }while (false)

#define NOLOG(message)\
    do{\
        static_cast<void>((true? static_cast<void>(0) : static_cast<void>((std::ostringstream() << message))));\
    }while(false)

#ifdef NDEBUG
#define LOG_V(message) NOLOG(message)
#define LOG_D(message) NOLOG(message)
#else
#define LOG_V(message) LOG(common::Logger::Severity::Verbose, message)
#define LOG_D(message) LOG(common::Logger::Severity::Debug, message)
#endif
#define LOG_I(message) LOG(common::Logger::Severity::Info, message)
#define LOG_W(message) LOG(common::Logger::Severity::Warning, message)
#define LOG_E(message) LOG(common::Logger::Severity::Error, message)
#define LOG_F(message) LOG(common::Logger::Severity::Fatal, message)