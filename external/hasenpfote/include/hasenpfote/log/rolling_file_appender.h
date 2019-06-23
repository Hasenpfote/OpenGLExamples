/*!
* @file rolling_file_appender.h
* @brief Log appender for file.
* @author Hasenpfote
* @date 2016/07/03
*/
#pragma once
#include <fstream>
#include <memory>
#if (__cplusplus > 201402L) || (defined(_MSC_VER) && (_MSVC_LANG > 201402L))
#include <filesystem>
#endif
#include "appender.h"

namespace hasenpfote{ namespace log{

class RollingFileAppender final : public IAppender
{
public:
    explicit RollingFileAppender(const std::string& filepath, int max_files = 1, std::size_t max_file_size = 1024);
#if (__cplusplus > 201402L) || (defined(_MSC_VER) && (_MSVC_LANG > 201402L))
    explicit RollingFileAppender(const std::filesystem::path& filepath, int max_files = 1, std::size_t max_file_size = 1024);
#endif
    ~RollingFileAppender() = default;
    void Write(const std::string& buffer) override;

private:
    std::unique_ptr<std::ofstream> ofs;
#if (__cplusplus > 201402L) || (defined(_MSC_VER) && (_MSVC_LANG > 201402L))
    std::filesystem::path filepath;
#else
    std::string filepath;
#endif
    int max_files;
    std::size_t max_file_size;
};

}}