#include <stdexcept>
#include <functional>
#include <hasenpfote/assert.h>
#include "../logger.h"
#include "image.h"
#include "texture.h"

namespace
{

enum class ColorSpace
{
    Linear,
    SRGB
};

bool ends_with_ignore_case(const std::string& s, const std::string& suffix)
{
    if(s.size() < suffix.size())
        return false;

    return std::equal(
        std::rbegin(suffix),
        std::rend(suffix),
        std::rbegin(s),
        [](const char& c1, const char& c2)
        {
            return ((c1 == c2) || (std::toupper(c1) == std::toupper(c2)));
        }
    );
}

}

namespace common::render
{

Texture::Texture(GLsizei levels, GLenum internal_format, GLsizei width, GLsizei height)
    : texture_(0)
{
    HASENPFOTE_ASSERT(levels > 0);

    LOG_I("Creating texture.");

    GLuint texture = 0;
    GLenum target = GL_TEXTURE_2D;

    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    glTexStorage2D(target, levels, internal_format, width, height);
#if 0
    // Poor filtering
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels - 1);

    glBindTexture(target, 0);

    LOG_I("Texture created successfully. [id=" << texture << "]");

    texture_ = texture;
}

Texture::Texture(GLenum internal_format, GLsizei width, GLsizei height)
    : Texture(1, internal_format, width, height)
{
}

Texture::Texture(const std::filesystem::path& filepath, bool generate_mipmap)
    : texture_(0)
{
    LOG_I("Creating texture from file `" << filepath.string() << "`.");

    Image image;
    if(!image.LoadFromFile(filepath))
    {
        LOG_E("Failed to load image from file `" << filepath.string() << "`.");
        throw std::runtime_error("");
    }

    ColorSpace color_space =
        (ends_with_ignore_case(filepath.stem().string(), "_linear")) ? ColorSpace::Linear : ColorSpace::SRGB;

    GLuint texture = 0;
    GLenum target = GL_TEXTURE_2D;
    GLenum format;
    GLint internal_format;
    GLenum type;
    GLint alignment;

    if(image.GetColorFormat() == Image::ColorFormat::R)
    {
        internal_format = GL_RED;
        format = GL_RED;
        alignment = 1;
    }
    else if(image.GetColorFormat() == Image::ColorFormat::RG)
    {
        internal_format = GL_RG;
        format = GL_RG;
        alignment = 2;
    }
    else if(image.GetColorFormat() == Image::ColorFormat::RGB)
    {
        format = GL_RGB;
        if(image.GetPixelType() == Image::PixelType::UnsignedByte)
        {
            internal_format =
                (color_space == ColorSpace::Linear) ? GL_RGB : GL_SRGB8;
            type = GL_UNSIGNED_BYTE;
            alignment = 1;
        }
        else if(image.GetPixelType() == Image::PixelType::Half)
        {
            internal_format = GL_RGB16F;
            type = GL_HALF_FLOAT;
            alignment = 2;
        }
        else if(image.GetPixelType() == Image::PixelType::Float)
        {
            internal_format = GL_RGB32F;
            type = GL_FLOAT;
            alignment = 4;
        }
        else
        {
            HASENPFOTE_ASSERT(false);
        }
    }
    else if(image.GetColorFormat() == Image::ColorFormat::RGBA)
    {
        format = GL_RGBA;
        if(image.GetPixelType() == Image::PixelType::UnsignedByte)
        {
            internal_format =
                (color_space == ColorSpace::Linear) ? GL_RGBA : GL_SRGB8_ALPHA8;
            type = GL_UNSIGNED_BYTE;
            alignment = 4;
        }
        else if(image.GetPixelType() == Image::PixelType::Half)
        {
            internal_format = GL_RGBA16F;
            type = GL_HALF_FLOAT;
            alignment = 8;
        }
        else if(image.GetPixelType() == Image::PixelType::Float)
        {
            internal_format = GL_RGBA32F;
            type = GL_FLOAT;
            alignment = 4;
        }
        else
        {
            HASENPFOTE_ASSERT(false);
        }
    }
    else
    {
        HASENPFOTE_ASSERT(false);
    }

    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glTexImage2D(target, 0, internal_format, image.GetWidth(), image.GetHeight(), 0, format, type, image.GetData());

    GLsizei levels = 1;
    if(generate_mipmap)
    {
#if 0
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#endif
        glGenerateMipmap(target);
        levels = CalcNumOfMipmapLevels(image.GetWidth(), image.GetHeight());
    }
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels - 1);

    glBindTexture(target, 0);

    LOG_I("Texture created successfully. [id=" << texture << "]");

    texture_ = texture;
}

Texture::~Texture()
{
    if(glIsTexture(texture_))
        glDeleteTextures(1, &texture_);
}

GLsizei Texture::CalcNumOfMipmapLevels(GLsizei width)
{
    return static_cast<GLsizei>(std::log2(static_cast<float>(width))) + 1;
}

GLsizei Texture::CalcNumOfMipmapLevels(GLsizei width, GLsizei height)
{
    return static_cast<GLsizei>(std::log2(static_cast<float>(std::max(width, height)))) + 1;
}

}   // namespace common::render