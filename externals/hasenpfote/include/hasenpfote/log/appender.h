/*!
* @file appender.h
* @brief Log appender interface.
* @author Hasenpfote
* @date 2016/07/03
*/
#pragma once
#include <string>

namespace hasenpfote{ namespace log{

class IAppender
{
public:
    IAppender() = default;
    virtual ~IAppender() = default;

    IAppender(const IAppender&) = delete;
    IAppender& operator = (const IAppender&) = delete;
    IAppender(IAppender&&) = delete;
    IAppender& operator = (IAppender&&) = delete;

    virtual void Write(const std::string& buffer) = 0;
};

}}