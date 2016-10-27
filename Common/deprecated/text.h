/*!
* @file text.h
* @brief Text renderer and related classes.
* @author Hasenpfote
* @date 2016/08/05
*/
#pragma once
#include <cinttypes>
#include <filesystem>
#include <unordered_map>
#include <tuple>
#include <memory>
#include <GL/glew.h>
#include <hasenpfote/range/range.h>

namespace text{

/*!
 * @class Glyph
 * @brief Represents a single glyph.
 */
class Glyph final
{
public:
    using Size = std::tuple<std::uint32_t, std::uint32_t>;
    using Bearing = std::tuple<std::int32_t, std::int32_t>;
    using TexCoord = std::tuple<float, float>;

public:
    Glyph(std::uint32_t advance, const Size& size, const Bearing& bearing, const TexCoord& texcoord);
    ~Glyph() = default;

    Glyph(const Glyph&) = default;
    Glyph& operator = (const Glyph&) = default;
    Glyph(Glyph&&) = delete;
    Glyph& operator = (Glyph&&) = delete;

    std::uint32_t GetAdvance() const;
    Size GetSize() const;
    Bearing GetBearing() const;
    TexCoord GetTexCoord() const;
    bool HasPixels() const;

private:
    std::uint32_t advance;  // Offset to advance to next glyph.
    Size size;              // Size of glyph.
    Bearing bearing;        // Offset from baseline to left/top of glyph.
    TexCoord texcoord;
};

/*!
 * @class FontTextureAtlas
 * @brief Generate bitmap fonts from TrueType fonts. 
 */
class FontTextureAtlas final
{
public:
    using Size = std::tuple<std::uint32_t, std::uint32_t>;
    using GlyphMap = std::unordered_map<std::uint32_t, Glyph>;
    using CharacterRange = hasenpfote::range::range<std::uint32_t, false>;

public:
    FontTextureAtlas();
    ~FontTextureAtlas();

    FontTextureAtlas(const FontTextureAtlas&) = delete;
    FontTextureAtlas& operator = (const FontTextureAtlas&) = delete;
    FontTextureAtlas(FontTextureAtlas&&) = delete;
    FontTextureAtlas& operator = (FontTextureAtlas&&) = delete;

    GLuint GetTexture() const;
    Size GetSize() const;
    const Glyph* GetGlyph(std::uint32_t code) const;

    bool Generate(const std::tr2::sys::path& filepath, std::uint32_t height, std::uint32_t max_width, const std::vector<CharacterRange>& ranges);
    void Release();

private:
    GLuint texture;
    Size size;
    GlyphMap glyph;
};

/*!
 * @class IRenderer
 * @brief 
 */
class IRenderer
{
    friend class Text;
public:
    using Rect = std::tuple<float, float, float, float>;    // left, top, right, bottom

public:
    IRenderer() = default;
    virtual ~IRenderer() = default;

    IRenderer(const IRenderer&) = delete;
    IRenderer& operator = (const IRenderer&) = delete;
    IRenderer(IRenderer&&) = delete;
    IRenderer& operator = (IRenderer&&) = delete;

protected:
    virtual void BeginRendering(GLuint texture) = 0;
    virtual void EndRendering() = 0;
    virtual void Render() = 0;

    virtual void SetOrthographicProjectionMatrix(const GLfloat* m) = 0;
    virtual void SetColor(const GLfloat* color) = 0;

    virtual std::size_t GetMaxBufferLength() = 0;
    virtual bool IsBufferEmpty() = 0;
    virtual void SetToBuffer(const Rect& pos, const Rect& texcoord) = 0;
    virtual void ClearBuffer() = 0;
};

/*!
 * @class Text
 * @brief Text renderer.
 */
class Text final
{
public:
    Text(const std::shared_ptr<FontTextureAtlas>& atlas, const std::shared_ptr<IRenderer>& renderer);
    ~Text() = default;

public:
    Text(const Text&) = delete;
    Text& operator = (const Text&) = delete;
    Text(Text&&) = delete;
    Text& operator = (Text&&) = delete;

    void BeginRendering();
    void EndRendering();
    void DrawString(const std::string& string, float x, float y, float scale = 1.0f);
    void SetOrthographicProjectionMatrix(const GLfloat* m);
    void SetColor(const GLfloat* color);

private:
    std::shared_ptr<const FontTextureAtlas> atlas;
    std::shared_ptr<IRenderer> renderer;
};

}