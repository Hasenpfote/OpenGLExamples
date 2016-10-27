/*!
* @file console_appender.h
* @brief Log appender for console.
* @author Hasenpfote
* @date 2016/07/03
*/
#pragma once
#include "appender.h"

namespace hasenpfote{ namespace log{

class ConsoleAppender final : public IAppender
{
public:
    ConsoleAppender() = default;
    ~ConsoleAppender() = default;
    void Write(const std::string& buffer) override;
};

}}