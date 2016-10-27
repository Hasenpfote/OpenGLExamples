/*!
* @file unicode.h
* @brief unicode helper.
* @author Hasenpfote
* @date 2016/09/12
*/
#pragma once
#include <locale>
#include <codecvt>

namespace hasenpfote{

std::u16string utf8_to_utf16(const std::string& s);
std::string utf16_to_utf8(const std::u16string& s);
std::u32string utf8_to_utf32(const std::string& s);
std::string utf32_to_utf8(const std::u32string& s);
std::u32string utf16_to_utf32(const std::u16string &s, bool is_big_endian = true);
std::u16string utf32_to_utf16(const std::u32string& s, bool is_big_endian = true);
std::string ucs2_to_utf8(const std::u16string& s);
std::u16string utf8_to_ucs2(const std::string& s);
std::u16string ucs2_to_utf16(const std::u16string &s, bool is_big_endian = true);
std::u16string utf16_to_ucs2(const std::u16string &s, bool is_big_endian = true);

}