/*!
* @file rolling_file_appender.h
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

class RollingFileAppender final : public IAppender
{
public:
    explicit RollingFileAppender(const std::tr2::sys::path& filepath, int max_files = 1, std::size_t max_file_size = 1024);
    ~RollingFileAppender() = default;
    void Write(const std::string& buffer) override;

private:
    std::unique_ptr<std::ofstream> ofs;
    std::tr2::sys::path filepath;
    std::size_t max_file_size;
    int max_files;
};

}}