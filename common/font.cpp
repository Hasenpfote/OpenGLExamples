#include <assert.h>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include "image.h"
#include "fnt_parser.h"
#include "font.h"

namespace text{

static GLuint Create2DArrayTexture(GLsizei width, GLsizei height, const std::vector<std::filesystem::path>& filepaths)
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, width, height, filepaths.size());

    Image image;
    GLsizei depth = 0;
    for(auto filepath : filepaths)
    {
        if(!image.LoadFromFile(filepath) || (image.GetColorFormat() != Image::ColorFormat::RGBA))
        {
            glDeleteTextures(1, &texture);
            return 0;
        }
        auto alpha_channel = image.ExtractChannel(Image::Channel::Alpha);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, depth, image.GetHeight(), image.GetHeight(), 1, GL_RED, GL_UNSIGNED_BYTE, alpha_channel.get());
        depth++;
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    return texture;
}

// Glyph

Glyph::Glyph()
    : x(), y(), width(), height(), xoffset(), yoffset(), advance(), page()
{
}

Glyph::Glyph(const UI16Duplet& position, const UI16Duplet& size, const I16Duplet& offset, std::int16_t advance, std::uint16_t page)
    : x(std::get<0>(position)), y(std::get<1>(position)),
      width(std::get<0>(size)), height(std::get<1>(size)),
      xoffset(std::get<0>(offset)), yoffset(std::get<1>(offset)),
      advance(advance), page(page)
{
}

Glyph::UI16Duplet Glyph::GetPosition() const
{
    return UI16Duplet(x, y);
}

Glyph::UI16Duplet Glyph::GetSize() const
{
    return UI16Duplet(width, height);
}

Glyph::I16Duplet Glyph::GetOffset() const
{
    return I16Duplet(xoffset, yoffset);
}

std::int16_t Glyph::GetAdvance() const
{
    return advance;
}

std::uint16_t Glyph::GetPage() const
{
    return page;
}

bool Glyph::HasPixels() const
{
    return (width > 0) && (height > 0);
}

bool Glyph::IsValid() const
{
    // return (HasPixels() && (advance > 0));
    return advance > 0;
}

// Font::Impl

class Font::Impl final
{
public:
    using GlyphMap = std::unordered_map<std::uint32_t, Glyph>;
    using KerningAmount = std::tuple<std::uint32_t, std::int16_t>;

public:
    Impl() = delete;
    explicit Impl(const std::filesystem::path& filepath);
    ~Impl();

    GLuint GetTexture() const { return texture; }
    Size GetTextureSize() const { return texture_size; }
    std::uint16_t GetBase() const { return base; }
    std::uint16_t GetLineHeight() const { return line_height; }
    const GlyphMap& GetGlyphMap() const {return glyph; }
    Glyph GetGlyph(std::uint32_t code) const;
    std::int16_t GetKerningAmount(std::uint32_t first, std::uint32_t second) const;

private:
    void Create(const std::filesystem::path& filepath);
    static bool Compare(const KerningAmount& lhs, const KerningAmount& rhs);

private:
    GLuint texture;
    Size texture_size;
    std::uint16_t base;
    std::uint16_t line_height;
    GlyphMap glyph;
    using KerningPairMap = std::unordered_map<std::uint32_t, std::vector<KerningAmount>>;
    KerningPairMap kerning_pair;
};

Font::Impl::Impl(const std::filesystem::path& filepath)
    : texture(0)
{
    Create(filepath);
}

Font::Impl::~Impl()
{
    if(glIsTexture(texture))
        glDeleteTextures(1, &texture);
}

Glyph Font::Impl::GetGlyph(std::uint32_t code) const
{
    decltype(glyph)::const_iterator it = glyph.find(code);
#if defined(_MSC_VER)
    static Glyph invalid;
    return Glyph((it != glyph.cend())? it->second : invalid);
#else
    return (it != glyph.cend()? it->second : Glyph();
#endif
}

std::int16_t Font::Impl::GetKerningAmount(std::uint32_t first, std::uint32_t second) const
{
    decltype(kerning_pair)::const_iterator it = kerning_pair.find(first);
    if(it != kerning_pair.cend()){
        const auto key = std::make_tuple(second, 0);
        auto lb_it = std::lower_bound(it->second.cbegin(), it->second.cend(), key, Compare);
        if((lb_it != it->second.cend()) && !Compare(key, *lb_it)){
            return std::get<1>(*lb_it);
        }
    }
    return 0;
}

void Font::Impl::Create(const std::filesystem::path& filepath)
{
    std::ifstream ifs(filepath.string(), std::ios::in | std::ios::binary);
    if(ifs.fail()){
        throw std::runtime_error("Failed to open file `" + filepath.string() + "`.");
    }

    std::string field;

    // <info>
    field = FntParser::ReadLine(ifs, FntParser::INFO_TAG);
    //auto info = FntParser::ParseInfo(field);

    // <common>
    field = FntParser::ReadLine(ifs, FntParser::COMMON_TAG);
    const auto common = FntParser::ParseCommon(field);

    // <page>
    const auto parent_path = filepath.parent_path();
    const auto num_pages = static_cast<int>(std::get<FntParser::as_integer(FntParser::CommonElement::Pages)>(common));
    std::vector<std::filesystem::path> filepaths;
    for(auto i = 0; i < num_pages; i++){
        field = FntParser::ReadLine(ifs, FntParser::PAGE_TAG);
        const auto page = FntParser::ParsePage(field);
        filepaths.emplace_back(parent_path / std::get<FntParser::as_integer(FntParser::PageElement::File)>(page));
    }

    // <chars>
    field = FntParser::ReadLine(ifs, FntParser::CHARS_TAG);
    const auto num_chars = FntParser::ParseChars(field);
    assert(num_chars > 0);
    GlyphMap glyph;
    glyph.reserve(num_chars);

    // <char>
    for(std::uint32_t i = 0; i < num_chars; i++){
        field = FntParser::ReadLine(ifs, FntParser::CHAR_TAG);
        const auto c = FntParser::ParseChar(field);
        glyph.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(std::get<FntParser::as_integer(FntParser::CharElement::Id)>(c)),
            std::forward_as_tuple(
                std::forward_as_tuple(
                    std::get<FntParser::as_integer(FntParser::CharElement::X)>(c),
                    std::get<FntParser::as_integer(FntParser::CharElement::Y)>(c)
                ),
                std::forward_as_tuple(
                    std::get<FntParser::as_integer(FntParser::CharElement::Width)>(c),
                    std::get<FntParser::as_integer(FntParser::CharElement::Height)>(c)
                ),
                std::forward_as_tuple(
                    std::get<FntParser::as_integer(FntParser::CharElement::XOffset)>(c),
                    std::get<FntParser::as_integer(FntParser::CharElement::YOffset)>(c)
                ),
                std::get<FntParser::as_integer(FntParser::CharElement::XAdvance)>(c),
                std::get<FntParser::as_integer(FntParser::CharElement::Page)>(c)
            )
        );
    }
    assert(num_chars == glyph.size());

    // <kernings>
    field = FntParser::ReadLine(ifs, FntParser::KERNINGS_TAG);
    const auto num_kernings = FntParser::ParseKernings(field);

    // <kerning>
    KerningPairMap kerning_pair;
    for(std::uint32_t i = 0; i < num_kernings; i++){
        field = FntParser::ReadLine(ifs, FntParser::KERNING_TAG);
        const auto kerning = FntParser::ParseKerning(field);
        //
        auto it = kerning_pair.find(std::get<FntParser::as_integer(FntParser::KerningElement::First)>(kerning));
        if(it == kerning_pair.cend()){
            it = kerning_pair.insert(
                std::make_pair(
                    std::get<FntParser::as_integer(FntParser::KerningElement::First)>(kerning),
                    std::vector<std::tuple<std::uint32_t, std::int16_t>>()
                )
            ).first;
        }
        it->second.emplace_back(
            std::make_tuple(
                std::get<FntParser::as_integer(FntParser::KerningElement::Second)>(kerning),
                std::get<FntParser::as_integer(FntParser::KerningElement::Amount)>(kerning)
            )
        );
    }

    // eof
    std::getline(ifs, field);
    if(!ifs.eof()){
        throw std::runtime_error("");
    }

    // Setup
    const auto scale_w = std::get<FntParser::as_integer(FntParser::CommonElement::ScaleW)>(common);
    const auto scale_h = std::get<FntParser::as_integer(FntParser::CommonElement::ScaleH)>(common);
    auto texture = Create2DArrayTexture(scale_w, scale_h, filepaths);
    if(!glIsTexture(texture)){
        throw std::runtime_error("Failed to create the texture.");
    }
    this->texture = texture;
    texture_size = std::make_tuple(scale_w, scale_h);
    this->base = std::get<FntParser::as_integer(FntParser::CommonElement::Base)>(common);
    line_height = std::get<FntParser::as_integer(FntParser::CommonElement::LineHeight)>(common);
    this->glyph = std::move(glyph);
    // for binary search.
    for(auto& pair : kerning_pair){
        std::sort(pair.second.begin(), pair.second.end(), Compare);
    }
    this->kerning_pair = std::move(kerning_pair);
}

bool Font::Impl::Compare(const KerningAmount& lhs, const KerningAmount& rhs)
{
    return std::get<0>(lhs) < std::get<0>(rhs);
}

// Font

Font::Font(const std::filesystem::path& filepath)
    : pimpl(std::make_shared<Impl>(filepath)), metrics(std::make_unique<FontMetrics>(*this))
{
}

Font::~Font() = default;

GLuint Font::GetTexture() const
{
    return pimpl->GetTexture();
}

Font::Size Font::GetTextureSize() const
{
    return pimpl->GetTextureSize();;
}

std::uint16_t Font::GetBase() const
{
    return pimpl->GetBase();
}

Glyph Font::GetGlyph(std::uint32_t code) const
{
    return pimpl->GetGlyph(code);
}

std::int16_t Font::GetKerningAmount(std::uint32_t first, std::uint32_t second) const
{
    return pimpl->GetKerningAmount(first, second);
}

FontMetrics Font::GetFontMetrics() const
{
    return *metrics.get();
}

// FontMetrics

FontMetrics::FontMetrics(const Font& font)
    : font_impl(font.pimpl), ascent(), descent(), line_gap(), line_height()
{
    auto sp = font_impl.lock();
    if(!sp)
        throw std::runtime_error("Failed to construct `FontMetrics`.");

    const auto base = static_cast<std::int32_t>(sp->GetBase());
    const auto line_h = static_cast<std::int32_t>(sp->GetLineHeight());
    std::int32_t ascent = 0;
    std::int32_t descent = 0;
    for(const auto& pair : sp->GetGlyphMap()){
        //auto key = pair.first;
        const auto& glyph = pair.second;
        const auto temp = base - std::get<1>(glyph.GetOffset());
        ascent = std::max(ascent, temp);
        descent = std::min(descent, temp - std::get<1>(glyph.GetSize()));
    }
    this->ascent = ascent;
    this->descent = std::abs(descent);
    const auto cell_height = this->ascent + this->descent;
    line_height = (line_h < cell_height) ? cell_height : line_h;
    line_gap = line_height - cell_height;
}

std::int32_t FontMetrics::GetAscent() const
{
    return ascent;
}

std::int32_t FontMetrics::GetDescent() const
{
    return descent;
}

std::int32_t FontMetrics::GetLineGap() const
{
    return line_gap;
}

std::int32_t FontMetrics::GetLineHeight() const
{
    return line_height;
}

std::int32_t FontMetrics::MeasureWidth(const std::u16string& string) const
{
    auto sp = font_impl.lock();
    if(!sp)
        return 0;

    std::int32_t width = 0;
    std::int32_t adjust = 0;
    char16_t prev_code = 0;
    for(auto code : string){
        auto glyph = sp->GetGlyph(code);
        if(!glyph.IsValid())
            continue;

        const auto advance = static_cast<std::int32_t>(glyph.GetAdvance());
        width += advance + sp->GetKerningAmount(prev_code, code);
        prev_code = code;

        adjust = std::get<0>(glyph.GetOffset());
        if(glyph.HasPixels()){
            adjust += std::get<0>(glyph.GetSize()) - advance;
        }
    }
    return width + adjust;
}

}