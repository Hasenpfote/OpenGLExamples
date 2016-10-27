/*!
* @file file_appender.h
* @brief Log appender for file.
* @author Hasenpfote
* @date 2016/07/03
*/
#pragma once
#include <fstream>
#include <memory>
#include <filesystem>
#include "appender.h"

namespace hasenpfote{ namespace log{

class FileAppender final : public IAppender
{
public:
    explicit FileAppender(const std::tr2::sys::path& filepath);
    ~FileAppender() = default;
    void Write(const std::string& buffer) override;

private:
    std::unique_ptr<std::ofstream> ofs;
};

}}