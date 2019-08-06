/*!
* @file font.h
* @brief Font and related classes.
* @author Hasenpfote
* @date 2016/09/26
*/
#pragma once
#include <cinttypes>
#include <tuple>
#include <filesystem>
#include <unordered_map>
#include <GL/glew.h>

namespace common::render::text
{

/*!
 * @class Glyph
 * @brief Represents a single glyph.
 */
class Glyph final
{
public:
    using I16Duplet = std::tuple<std::int16_t, std::int16_t>;
    using UI16Duplet = std::tuple<std::uint16_t, std::uint16_t>;

public:
    Glyph();
    Glyph(const UI16Duplet& position, const UI16Duplet& size, const I16Duplet& offset, std::int16_t advance, std::uint16_t page);
    ~Glyph() = default;

    Glyph(const Glyph&) = default;
    Glyph& operator = (const Glyph&) = default;
    Glyph(Glyph&&) = default;
    Glyph& operator = (Glyph&&) = default;

    UI16Duplet GetPosition() const;
    UI16Duplet GetSize() const;
    I16Duplet GetOffset() const;
    std::int16_t GetAdvance() const;
    std::uint16_t GetPage() const;
    bool HasPixels() const;
    bool IsValid() const;

private:
    std::uint16_t x;
    std::uint16_t y;
    std::uint16_t width;
    std::uint16_t height;
    std::int16_t xoffset;
    std::int16_t yoffset;
    std::int16_t advance;
    std::uint16_t page;
};

/*!
 * @class Font
 * @brief Generate bitmap fonts from TrueType fonts.
 */
class Font final
{
    friend class FontMetrics;

public:
    using Size = std::tuple<std::uint16_t, std::uint16_t>;

public:
    Font() = delete;
    explicit Font(const std::filesystem::path& filepath);
    ~Font();

    Font(const Font&) = delete;
    Font& operator = (const Font&) = delete;
    Font(Font&&) = delete;
    Font& operator = (Font&&) = delete;

    GLuint GetTexture() const;
    Size GetTextureSize() const;
    std::uint16_t GetBase() const;
    Glyph GetGlyph(std::uint32_t code) const;
    std::int16_t GetKerningAmount(std::uint32_t first, std::uint32_t second) const;
    FontMetrics GetFontMetrics() const;

private:
    class Impl;
    std::shared_ptr<Impl> pimpl;
    std::unique_ptr<FontMetrics> metrics;
};

/*!
 * @class FontMetrics
 * @brief
 */
class FontMetrics final
{
public:
    FontMetrics() = delete;
    explicit FontMetrics(const Font& font);
    ~FontMetrics() = default;

    FontMetrics(const FontMetrics&) = default;
    FontMetrics& operator = (const FontMetrics&) = default;
    FontMetrics(FontMetrics&&) = default;
    FontMetrics& operator = (FontMetrics&&) = default;

    std::int32_t GetAscent() const;
    std::int32_t GetDescent() const;
    std::int32_t GetLineGap() const;
    std::int32_t GetLineHeight() const;
    std::int32_t MeasureWidth(const std::u16string& string) const;

private:
    std::weak_ptr<Font::Impl> font_impl;
    std::int32_t ascent;
    std::int32_t descent;
    std::int32_t line_gap;
    std::int32_t line_height;
};

}   // namespace common::text