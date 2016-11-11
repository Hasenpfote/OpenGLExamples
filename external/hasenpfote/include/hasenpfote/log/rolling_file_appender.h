/*!
* @file rolling_file_appender.h
* @brief Log appender for file.
* @author Hasenpfote
* @date 2016/07/03
*/
#pragma once
#include <fstream>
#include <memory>
#if (__cplusplus > 201402L) || (defined(_MSC_VER) && (_MSC_VER >= 1900))
#include <filesystem>
#endif
#include "appender.h"

namespace hasenpfote{ namespace log{

class RollingFileAppender final : public IAppender
{
public:
    explicit RollingFileAppender(const std::string& filepath, int max_files = 1, std::size_t max_file_size = 1024);
#if (__cplusplus > 201402L) || defined(_MSC_VER) && (_MSC_VER > 1900)
#error Function not implemented.
#elif defined(_MSC_VER) && (_MSC_VER == 1900)
    explicit RollingFileAppender(const std::tr2::sys::path& filepath, int max_files = 1, std::size_t max_file_size = 1024);
#endif
    ~RollingFileAppender() = default;
    void Write(const std::string& buffer) override;

private:
    std::unique_ptr<std::ofstream> ofs;
#if (__cplusplus > 201402L) || (defined(_MSC_VER) && (_MSC_VER > 1900))
#error Function not implemented.
    std::filesystem::path filepath;
#elif defined(_MSC_VER) && (_MSC_VER == 1900)
    std::tr2::sys::path filepath;
#else
    std::string filepath;
#endif
    int max_files;
    std::size_t max_file_size;
};

}}