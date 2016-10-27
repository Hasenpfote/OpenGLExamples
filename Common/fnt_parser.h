/*!
* @file fnt_parser.h
* @brief This utility class is used to parse an Angel Code bitmap font format(http://www.angelcode.com/products/bmfont/).
* @author Hasenpfote
* @date 2016/09/03
*/
#pragma once
#include <cinttypes>
#include <string>
#include <tuple>

namespace text{

class FntParser final
{
public:
    using UI8Duplet = std::tuple<std::uint8_t, std::uint8_t>;
    using UI8Quadruplet = std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>;

    using Info = std::tuple<
        std::string,    // face
        std::uint16_t,  // size
        std::uint8_t,   // bold
        std::uint8_t,   // italic
        std::string,    // charset
        std::uint8_t,   // unicode
        std::uint16_t,  // stretchH
        std::uint8_t,   // smooth
        std::uint8_t,   // aa
        UI8Quadruplet,  // padding
        UI8Duplet,      // spacing
        std::uint8_t>;  // outline

    using Common = std::tuple<
        std::uint16_t,  // lineHeight
        std::uint16_t,  // base
        std::uint16_t,  // scaleW
        std::uint16_t,  // scaleH
        std::uint16_t,  // pages
        std::uint8_t,   // packed
        std::uint8_t,   // alphaChnl
        std::uint8_t,   // redChnl 
        std::uint8_t,   // greenChnl
        std::uint8_t>;  // blueChnl

    using Page = std::tuple<
        std::uint8_t,   // id
        std::string>;   // file

    using Char = std::tuple<
        std::uint32_t,  // id
        std::uint16_t,  // x
        std::uint16_t,  // y
        std::uint16_t,  // width
        std::uint16_t,  // height
        std::int16_t,   // xoffset
        std::int16_t,   // yoffset
        std::int16_t,   // xadvance
        std::uint8_t,   // page
        std::uint8_t>;  // chnl

    using Kerning = std::tuple<
        std::uint32_t,  // first
        std::uint32_t,  // second
        std::int16_t>;  // amount

    enum class InfoElement
    {
        Face = 0,
        Size = 1,
        Bold = 2,
        Italic = 3,
        Charset = 4,
        Unicode = 5,
        StretchW = 6,
        StretchH = 7,
        Smooth = 8,
        Aa = 9,
        Padding = 10,
        Spacing = 11,
        Outline = 12
    };

    enum class CommonElement
    {
        LineHeight = 0,
        Base = 1,
        ScaleW = 2,
        ScaleH = 3,
        Pages = 4,
        Packed = 5
    };

    enum class PageElement
    {
        Id = 0,
        File = 1
    };

    enum class CharElement
    {
        Id = 0,
        X = 1,
        Y = 2,
        Width = 3,
        Height = 4,
        XOffset = 5,
        YOffset = 6,
        XAdvance = 7,
        Page = 8,
        Chnl = 9
    };

    enum class KerningElement
    {
        First = 0,
        Second = 1,
        Amount = 2
    };

    template <typename Enumeration>
    static constexpr auto as_integer(Enumeration e) -> typename std::underlying_type<Enumeration>::type
    {
        static_assert(std::is_enum<Enumeration>::value, "e is not of type enum or enum class.");
        return static_cast<typename std::underlying_type<Enumeration>::type>(e);
    }

    static const std::string INFO_TAG;
    static const std::string COMMON_TAG;
    static const std::string PAGE_TAG;
    static const std::string CHARS_TAG;
    static const std::string CHAR_TAG;
    static const std::string KERNINGS_TAG;
    static const std::string KERNING_TAG;

public:
    FntParser() = delete;
    ~FntParser() = delete;
    FntParser(const FntParser&) = delete;
    FntParser& operator=(const FntParser&) = delete;
    FntParser(FntParser&&) = delete;
    FntParser& operator=(FntParser&&) = delete;

    static std::string ReadLine(std::ifstream& ifs, const std::string& tag);

    static Info ParseInfo(const std::string& source);
    static Common ParseCommon(const std::string& source);
    static Page ParsePage(const std::string& source);
    static std::uint32_t ParseChars(const std::string& source);
    static Char ParseChar(const std::string& source);
    static std::uint32_t ParseKernings(const std::string& source);
    static Kerning ParseKerning(const std::string& source);
};

}