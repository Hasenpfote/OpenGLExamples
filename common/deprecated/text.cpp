#if 0
#include <cassert>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "text.h"

namespace text{

Glyph::Glyph(std::uint32_t advance, const Size& size, const Bearing& bearing, const TexCoord& texcoord)
    : advance(advance), size(size), bearing(bearing), texcoord(texcoord)
{
}

std::uint32_t Glyph::GetAdvance() const
{
    return advance;
}

Glyph::Size Glyph::GetSize() const
{
    return size;
}

Glyph::Bearing Glyph::GetBearing() const
{
    return bearing;
}

Glyph::TexCoord Glyph::GetTexCoord() const
{
    return texcoord;
}

bool Glyph::HasPixels() const
{
    return (std::get<0>(size) > 0) && (std::get<1>(size) > 0);
}

static FontTextureAtlas::Size CalcMinimumTextureSize(FT_Face face, std::uint32_t padding, std::uint32_t max_width, const std::vector<FontTextureAtlas::CharacterRange>& ranges)
{
    if(ranges.empty())
        return std::make_tuple(0, 0);

    FT_GlyphSlot g = face->glyph;

    std::uint32_t row_w = padding;
    std::uint32_t row_h = 0;
    std::uint32_t w = 0;
    std::uint32_t h = padding;

    for(auto range : ranges){
        for(auto code : range){
            if(FT_Load_Char(face, code, FT_LOAD_RENDER)){
                std::cerr << "Loading character " << code << "failed." << std::endl;
                continue;
            }
            if((row_w + g->bitmap.width + padding) > max_width){
                w = std::max(w, row_w);
                h += row_h;
                row_w = padding;
                row_h = 0;
            }
            row_w += g->bitmap.width + padding;
            row_h = std::max(row_h, g->bitmap.rows + padding);
        }
    }
    w = std::max(w, row_w);
    h += row_h;
    return std::make_tuple(w, h);
}

static GLuint GenerateTexture(FT_Face face, std::uint32_t padding, std::uint32_t max_width, const std::vector<FontTextureAtlas::CharacterRange>& ranges, FontTextureAtlas::Size& size, FontTextureAtlas::GlyphMap& map)
{
    size = CalcMinimumTextureSize(face, padding, max_width, ranges);
    const auto w = std::get<0>(size);
    const auto h = std::get<1>(size);
    assert(w > 0 && h > 0);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    GLubyte clear_color = 0;
    //GLubyte clear_color = 255;    // For debug.
    glClearTexImage(texture, 0, GL_RED, GL_UNSIGNED_BYTE, &clear_color);

    FT_GlyphSlot g = face->glyph;

    std::uint32_t row_h = 0;
    int offset_x = padding;
    int offset_y = padding;

    for(auto& range : ranges){
        for(auto code : range){
            if(FT_Load_Char(face, code, FT_LOAD_RENDER)){
                std::cerr << "Loading character " << code << "failed." << std::endl;
                continue;
            }
            if((offset_x + g->bitmap.width + padding) > max_width){
                offset_x = padding;
                offset_y += row_h;
                row_h = 0;
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
#if 0
            Glyph glyph(
                g->advance.x >> 6,  // 1/64
                std::make_tuple(g->bitmap.width, g->bitmap.rows),
                std::make_tuple(g->bitmap_left, g->bitmap_top),
                std::make_tuple(static_cast<float>(offset_x) / static_cast<float>(w), static_cast<float>(offset_y) / static_cast<float>(h))
            );
            map.insert(std::make_pair(code, glyph));
#else
            map.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(code),
                std::forward_as_tuple(
                    g->advance.x >> 6,  // 1/64
                    std::forward_as_tuple(g->bitmap.width, g->bitmap.rows),
                    std::forward_as_tuple(g->bitmap_left, g->bitmap_top),
                    std::forward_as_tuple(static_cast<float>(offset_x) / static_cast<float>(w), static_cast<float>(offset_y) / static_cast<float>(h)))
            );
#endif
            row_h = std::max(row_h, g->bitmap.rows + padding);
            offset_x += g->bitmap.width + padding;
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

static std::vector<FontTextureAtlas::CharacterRange> RemoveOverlaps(std::vector<FontTextureAtlas::CharacterRange> ranges)
{
    //
    std::sort(
        ranges.begin(),
        ranges.end(),
        [](const FontTextureAtlas::CharacterRange& lhs, const FontTextureAtlas::CharacterRange& rhs) -> bool
        {
            if(*lhs.begin() < *rhs.begin())
                return true;
            if(*lhs.begin() > *rhs.begin())
                return false;
            return *lhs.end() < *rhs.end();
        }
    );
    //
    ranges.erase(
        std::unique(
            ranges.begin(),
            ranges.end(),
            [](const FontTextureAtlas::CharacterRange& lhs, const FontTextureAtlas::CharacterRange& rhs) -> bool
            {
                if(*lhs.begin() != *rhs.begin())
                    return false;
                if(*lhs.end() != *rhs.end())
                    return false;
                return true;
            }
        ),
        ranges.end()
    );
    //
    std::vector<FontTextureAtlas::CharacterRange> result;
    result.reserve(ranges.size());
    for(auto current = ranges.begin(); current != ranges.end(); ++current){
        auto b = *current->begin();
        auto e = *current->end();
        for(auto next = current + 1; next != ranges.end(); ++next){
            if(*next->end() <= e){
                current = next;
            }
            else
            if(*next->begin() <= e){
                e = *next->end();
                current = next;
            }
            else{
                break;
            }
        }
        result.emplace_back(b, e);
    }
    result.shrink_to_fit();
    return result;
}

FontTextureAtlas::FontTextureAtlas()
    : texture(0)
{
}

FontTextureAtlas::~FontTextureAtlas()
{
    if(glIsTexture(texture))
        glDeleteTextures(1, &texture);
}

GLuint FontTextureAtlas::GetTexture() const
{
    return texture;
}

FontTextureAtlas::Size FontTextureAtlas::GetSize() const
{
    return size;
}

const Glyph* FontTextureAtlas::GetGlyph(std::uint32_t code) const
{
    const auto& it = glyph.find(code);
    if(it != glyph.cend())
        return &it->second;
    assert(!"Not found.");
    return nullptr;
}

bool FontTextureAtlas::Generate(const std::tr2::sys::path& filepath, std::uint32_t height, std::uint32_t max_width, const std::vector<CharacterRange>& ranges)
{
    Release();

    FT_Error error;

    FT_Library library;
    error = FT_Init_FreeType(&library);
    if(error){
        std::cerr << "Could not initialize freetype library. (code=" << error << ")" << std::endl;
        return false;
    }

    FT_Face face;
    error = FT_New_Face(library, filepath.string().c_str(), 0, &face);
    if(error){
        std::cerr << "Could not open font. (code=" << error << ")" << std::endl;
        error = FT_Done_FreeType(library);
        assert(error == 0);
        return false;
    }

    error = FT_Set_Pixel_Sizes(face, 0, height);
    assert(error == 0);

    static constexpr std::uint32_t padding = 1;
#if 0
    texture = GenerateTexture(face, padding, max_width, ranges, size, glyph);
#else
    auto copy = ranges;
    auto new_ranges = RemoveOverlaps(std::move(copy));
    texture = GenerateTexture(face, padding, max_width, new_ranges, size, glyph);
#endif
    assert(glIsTexture(texture));
    std::cout << "Generated a " << std::get<0>(size) << " x " << std::get<1>(size);
    std::cout << " (" << (std::get<0>(size) * std::get<1>(size) / 1024) << " KB) texture atlas." << std::endl;

    error = FT_Done_Face(face);
    assert(error == 0);

    error = FT_Done_FreeType(library);
    assert(error == 0);

    return true;
}

void FontTextureAtlas::Release()
{
    if(glIsTexture(texture)){
        glDeleteTextures(1, &texture);
    }
    texture = 0;
    size = std::make_tuple(0, 0);
    glyph.clear();
}


Text::Text(const std::shared_ptr<FontTextureAtlas>& atlas, const std::shared_ptr<IRenderer>& renderer)
    : atlas(atlas), renderer(renderer)
{
}

void Text::BeginRendering()
{
    renderer->BeginRendering(atlas->GetTexture());
}

void Text::EndRendering()
{
    renderer->EndRendering();
}

void Text::DrawString(const std::string& string, float x, float y, float scale)
{
    const auto size = atlas->GetSize();
    const auto aw = std::get<0>(size);
    const auto ah = std::get<1>(size);
    const std::size_t max_buffer_length = renderer->GetMaxBufferLength();

    std::size_t count = 0;
    for(auto code : string){
        auto glyph = atlas->GetGlyph(code);
        if(!glyph)
            continue;
        /* Skip glyphs that have no pixels. */
        if(glyph->HasPixels()){
            /* Calculate the vertex and texture coordinates */
            const float x2 = x + static_cast<float>(std::get<0>(glyph->GetBearing())) * scale;
            const float y2 = y - static_cast<float>(std::get<1>(glyph->GetBearing())) * scale;
            const auto bw = static_cast<float>(std::get<0>(glyph->GetSize()));
            const auto bh = static_cast<float>(std::get<1>(glyph->GetSize()));
            const auto tx = static_cast<float>(std::get<0>(glyph->GetTexCoord()));
            const auto ty = static_cast<float>(std::get<1>(glyph->GetTexCoord()));
            renderer->SetToBuffer(
                std::make_tuple(x2, y2, x2 + (bw * scale), y2 + (bh * scale)),
                std::make_tuple(tx, ty, tx + bw / aw, ty + bh / ah)
            );
            count++;
            if(count >= max_buffer_length){
                count = 0;
                renderer->Render();
                renderer->ClearBuffer();
            }
        }
        /* Advance the cursor to the start of the next character. */
        x += static_cast<float>(glyph->GetAdvance()) * scale;
    }
    if(!renderer->IsBufferEmpty()){
        renderer->Render();
        renderer->ClearBuffer();
    }
}

void Text::SetOrthographicProjectionMatrix(const GLfloat* m)
{
    renderer->SetOrthographicProjectionMatrix(m);
}

void Text::SetColor(const GLfloat* color)
{
    renderer->SetColor(color);
}

}
#endif