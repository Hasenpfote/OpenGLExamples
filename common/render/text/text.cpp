#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "text.h"

namespace common::render::text
{

Text::Text(const std::shared_ptr<Font>& font, const std::shared_ptr<ITextRenderer>& renderer)
    : font(font), renderer(renderer)
{
}

void Text::BeginRendering()
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    const auto proj = glm::ortho(
        static_cast<float>(vp[0]),
        static_cast<float>(vp[2]),
        static_cast<float>(vp[3]),
        static_cast<float>(vp[1]),
        -1.0f, 1.0f
    );
    renderer->SetOrthographicProjectionMatrix(glm::value_ptr(proj));

    renderer->BeginRendering(font->GetTexture());
}

void Text::EndRendering()
{
    renderer->EndRendering();
}

#ifdef _WIN32
#include <windows.h>

//
//  Convert an UTF8 string to a wide Unicode String
//  https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte/3999597#3999597
//
std::wstring utf8_decode(const std::string& str)
{
    if(str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
#endif

void Text::DrawString(const std::string& string, float x, float y, float scale)
{
#ifdef _WIN32
    // https://stackoverflow.com/a/42734882
    auto wstr = utf8_decode(string);
    std::u16string u16str(wstr.begin(), wstr.end());
    DrawString(u16str, x, y, scale);
#else
#error Function not implemented.
#endif
}

static std::tuple<float, float, float, float>
ComputeBounds(const Glyph& glyph, float x, float y, float scale)
{
    const auto x2 = x + static_cast<float>(std::get<0>(glyph.GetOffset())) * scale;
    const auto y2 = y + static_cast<float>(std::get<1>(glyph.GetOffset())) * scale;
    return std::make_tuple(x2, y2, x2 + (static_cast<float>(std::get<0>(glyph.GetSize())) * scale), y2 + (static_cast<float>(std::get<1>(glyph.GetSize())) * scale));
}

static std::tuple<float, float, float, float>
ComputeTexcoordBounds(const Glyph& glyph, float rcp_width, float rcp_height)
{
    const auto tx = (static_cast<float>(std::get<0>(glyph.GetPosition())) + 0.5f) * rcp_width;
    const auto ty = (static_cast<float>(std::get<1>(glyph.GetPosition())) + 0.5f) * rcp_height;
    return std::make_tuple(tx, ty, tx + (static_cast<float>(std::get<0>(glyph.GetSize())) * rcp_width), ty + (static_cast<float>(std::get<1>(glyph.GetSize())) * rcp_height));
}

void Text::DrawString(const std::u16string& string, float x, float y, float scale)
{
    const auto rcp_aw = 1.0f / static_cast<float>(std::get<0>(font->GetTextureSize()));
    const auto rcp_ah = 1.0f / static_cast<float>(std::get<1>(font->GetTextureSize()));
    const std::size_t max_buffer_length = renderer->GetMaxBufferLength();

    y -= static_cast<float>(font->GetBase()) * scale;

    char16_t prev_code = 0;
    std::size_t count = 0;
    for(auto code : string){
        auto glyph = font->GetGlyph(code);
        if(!glyph.IsValid())
            continue;

        // Adjust kerning.
        const auto amount = static_cast<float>(font->GetKerningAmount(prev_code, code));
        prev_code = code;
        x += amount * scale;

        // Skip glyphs that have no pixels.
        if(glyph.HasPixels()){
            // Calculate the vertex and texture coordinates.
            renderer->SetToBuffer(
                ComputeBounds(glyph, x, y, scale),
                ComputeTexcoordBounds(glyph, rcp_aw, rcp_ah),
                glyph.GetPage()
            );
            count++;
            if(count >= max_buffer_length){
                count = 0;
                renderer->Render();
                renderer->ClearBuffer();
            }
        }
        // Advance the cursor to the start of the next character.
        x += static_cast<float>(glyph.GetAdvance()) * scale;
    }
    if(!renderer->IsBufferEmpty()){
        renderer->Render();
        renderer->ClearBuffer();
    }
}

void Text::SetColor(const GLfloat* color)
{
    renderer->SetColor(color);
}

const Font& Text::GetFont() const
{
    return *font.get();
}

ITextRenderer* Text::GetRenderer()
{
    return renderer.get();
}

}   // namespace common::text