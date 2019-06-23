/*!
* @file file_appender.h
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

class FileAppender final : public IAppender
{
public:
    explicit FileAppender(const std::string& filepath);
#if (__cplusplus > 201402L) || (defined(_MSC_VER) && (_MSVC_LANG > 201402L))
    explicit FileAppender(const std::filesystem::path& filepath);
#endif
    ~FileAppender() = default;
    void Write(const std::string& buffer) override;

private:
    std::unique_ptr<std::ofstream> ofs;
};

}}