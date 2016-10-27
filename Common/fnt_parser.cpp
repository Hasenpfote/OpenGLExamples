#include <fstream>
#include <regex>
#include "fnt_parser.h"

namespace text{

const std::string FntParser::INFO_TAG = "info";
const std::string FntParser::COMMON_TAG = "common";
const std::string FntParser::PAGE_TAG = "page";
const std::string FntParser::CHARS_TAG = "chars";
const std::string FntParser::CHAR_TAG = "char";
const std::string FntParser::KERNINGS_TAG = "kernings";
const std::string FntParser::KERNING_TAG = "kerning";

template <typename T>
static T GetInteger(const std::string& source, const std::string& name, T default_value = 0)
{
    std::smatch match;
    if(std::regex_search(source, match, std::regex(name + R"(=([-]{0,1}[\d]+))")))
        return static_cast<T>(std::stoi(match[1])); // fnt ファイルで扱う分には問題ない数値となるが要注意.
    return default_value;
}

template <typename T>
static std::tuple<T, T> GetIntegerDuplet(const std::string& source, const std::string& name, std::tuple<T, T> default_value = { 0, 0 })
{
    std::smatch match;
    if(std::regex_search(source, match, std::regex(name + R"(=([-]{0,1}[\d]+),([-]{0,1}[\d]+))")))
        return std::make_tuple(
            static_cast<T>(std::stoi(match[1])),
            static_cast<T>(std::stoi(match[2])));
    return default_value;
}

template <typename T>
static std::tuple<T, T, T, T> GetIntegerQuadruplet(const std::string& source, const std::string& name, std::tuple<T, T, T, T> default_value = { 0, 0, 0, 0 })
{
    std::smatch match;
    if(std::regex_search(source, match, std::regex(name + R"(=([-]{0,1}[\d]+),([-]{0,1}[\d]+),([-]{0,1}[\d]+),([-]{0,1}[\d]+))")))
        return std::make_tuple(
            static_cast<T>(std::stoi(match[1])),
            static_cast<T>(std::stoi(match[2])),
            static_cast<T>(std::stoi(match[3])),
            static_cast<T>(std::stoi(match[4])));
    return default_value;
}

static bool GetBoolean(const std::string& source, const std::string& name, bool default_value = false)
{
    std::smatch match;
    if(std::regex_search(source, match, std::regex(name + R"(=([\d]))")))
        return std::stoi(match[1]) > 0;
    return default_value;
}

static std::string GetString(const std::string& source, const std::string& name)
{
    std::smatch match;
    if(std::regex_search(source, match, std::regex(name + R"(=\"([\w|\s|\+|\.|-]*)\")")))
        return match[1];
    return "";
}

std::string FntParser::ReadLine(std::ifstream& ifs, const std::string& tag)
{
    std::string field;
    if(!std::getline(ifs, field)){
        throw std::runtime_error("Failed to get line.");
    }
    if(!field.compare(0, tag.length(), tag) == 0){
        throw std::runtime_error("Does not match the tag.");
    }
    return field;
}

FntParser::Info FntParser::ParseInfo(const std::string& source)
{
    return std::make_tuple(
        GetString(source, "face"),
        GetInteger<std::uint16_t>(source, "size"),
        GetInteger<std::uint8_t>(source, "bold"),
        GetInteger<std::uint8_t>(source, "italic"),
        GetString(source, "charset"),
        GetInteger<std::uint8_t>(source, "unicode"),
        GetInteger<std::uint16_t>(source, "stretchH"),
        GetInteger<std::uint8_t>(source, "smooth"),
        GetInteger<std::uint8_t>(source, "aa"),
        GetIntegerQuadruplet<std::uint8_t>(source, "padding"),
        GetIntegerDuplet<std::uint8_t>(source, "spacing"),
        GetInteger<std::uint8_t>(source, "outline")
    );
}

FntParser::Common FntParser::ParseCommon(const std::string& source)
{
    return std::make_tuple(
        GetInteger<std::uint16_t>(source, "lineHeight"),
        GetInteger<std::uint16_t>(source, "base"),
        GetInteger<std::uint16_t>(source, "scaleW"),
        GetInteger<std::uint16_t>(source, "scaleH"),
        GetInteger<std::uint16_t>(source, "pages"),
        GetInteger<std::uint8_t>(source, "packed"),
        GetInteger<std::uint8_t>(source, "alphaChnl"),
        GetInteger<std::uint8_t>(source, "redChnl"),
        GetInteger<std::uint8_t>(source, "greenChnl"),
        GetInteger<std::uint8_t>(source, "blueChnl")
    );
}

FntParser::Page FntParser::ParsePage(const std::string& source)
{
    return std::make_tuple(
        GetInteger<std::uint8_t>(source, "id"),
        GetString(source, "file")
    );
}

std::uint32_t FntParser::ParseChars(const std::string& source)
{
    return GetInteger<std::uint32_t>(source, "count");
}

FntParser::Char FntParser::ParseChar(const std::string& source)
{
    static const std::string id_tag = "id=";
    static const std::string x_tag = "x=";
    static const std::string y_tag = "y=";
    static const std::string width_tag = "width=";
    static const std::string height_tag = "height=";
    static const std::string xoffset_tag = "xoffset=";
    static const std::string yoffset_tag = "yoffset=";
    static const std::string xadvance_tag = "xadvance=";
    static const std::string page_tag = "page=";
    static const std::string chnl_tag = "chnl=";

    FntParser::Char result;

    std::size_t start = 0;
    std::string delim = " ";
    auto end = source.find(delim);
    while(end != std::string::npos){
        auto sub = source.substr(start, end - start);
        if(!sub.empty()){
            if(sub.compare(0, id_tag.length(), id_tag) == 0){
                std::get<as_integer(CharElement::Id)>(result) = std::stoi(sub.substr(id_tag.length()));
            }
            else
            if(sub.compare(0, x_tag.length(), x_tag) == 0){
                std::get<as_integer(CharElement::X)>(result) = std::stoi(sub.substr(x_tag.length()));
            }
            else
            if(sub.compare(0, y_tag.length(), y_tag) == 0){
                std::get<as_integer(CharElement::Y)>(result) = std::stoi(sub.substr(y_tag.length()));
            }
            else
            if(sub.compare(0, width_tag.length(), width_tag) == 0){
                std::get<as_integer(CharElement::Width)>(result) = std::stoi(sub.substr(width_tag.length()));
            }
            else
            if(sub.compare(0, height_tag.length(), height_tag) == 0){
                std::get<as_integer(CharElement::Height)>(result) = std::stoi(sub.substr(height_tag.length()));
            }
            else
            if(sub.compare(0, xoffset_tag.length(), xoffset_tag) == 0){
                std::get<as_integer(CharElement::XOffset)>(result) = std::stoi(sub.substr(xoffset_tag.length()));
            }
            else
            if(sub.compare(0, yoffset_tag.length(), yoffset_tag) == 0){
                std::get<as_integer(CharElement::YOffset)>(result) = std::stoi(sub.substr(yoffset_tag.length()));
            }
            else
            if(sub.compare(0, xadvance_tag.length(), xadvance_tag) == 0){
                std::get<as_integer(CharElement::XAdvance)>(result) = std::stoi(sub.substr(xadvance_tag.length()));
            }
            else
            if(sub.compare(0, page_tag.length(), page_tag) == 0){
                std::get<as_integer(CharElement::Page)>(result) = std::stoi(sub.substr(page_tag.length()));
            }
            else
            if(sub.compare(0, chnl_tag.length(), chnl_tag) == 0){
                std::get<as_integer(CharElement::Chnl)>(result) = std::stoi(sub.substr(chnl_tag.length()));
            }
        }
        start = end + delim.length();
        end = source.find(delim, start);
    }
    return result;
}

std::uint32_t FntParser::ParseKernings(const std::string& source)
{
    return GetInteger<std::uint32_t>(source, "count");
}

FntParser::Kerning FntParser::ParseKerning(const std::string& source)
{
    return std::make_tuple(
        GetInteger<std::uint32_t>(source, "first"),
        GetInteger<std::uint32_t>(source, "second"),
        GetInteger<std::int16_t>(source, "amount")
    );
}

}