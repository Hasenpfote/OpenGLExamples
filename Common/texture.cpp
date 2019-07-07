#include <functional>
#include <hasenpfote/assert.h>
#include "logger.h"
#include "image.h"
#include "texture.h"

TextureManager::~TextureManager()
{
    DeleteTextures();
}

GLuint TextureManager::CreateTexture(const std::string& name, GLsizei levels, GLenum internal_format, GLsizei width, GLsizei height)
{
    HASENPFOTE_ASSERT(levels > 0);

    LOG_I("Creating texture file: " << name);
    auto texture = GetTexture(name);
    if(glIsTexture(texture))
    {
        LOG_E(name << ": Already exists.");
        return texture;
    }

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

    const auto hash = std::hash<std::string>()(name);
    this->texture.insert(std::make_pair(hash, texture));

    LOG_I(name << ": Succeed to create the texture. (id=" << texture << ")");

    return texture;
}

GLuint TextureManager::CreateTexture(const std::string& name, GLenum internal_format, GLsizei width, GLsizei height)
{
    return CreateTexture(name, 1, internal_format, width, height);
}

GLuint TextureManager::CreateTextureFromFile(const std::filesystem::path& filepath, bool generate_mipmap)
{
    LOG_I("Creating texture file: " << filepath.string());
    auto texture = GetTexture(filepath.string());
    if(glIsTexture(texture))
    {
        LOG_E(filepath.string() << ": Already exists.");
        return texture;
    }

    Image image;
    if(!image.LoadFromFile(filepath))
    {
        LOG_E(filepath.string() << ": Failed to load the file.");
        return 0;
    }

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
            internal_format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
            alignment = 1;
        }
        else if (image.GetPixelType() == Image::PixelType::Half)
        {
            internal_format = GL_RGB16F;
            type = GL_HALF_FLOAT;
            alignment = 2;
        }
        else if (image.GetPixelType() == Image::PixelType::Float)
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
            internal_format = GL_RGBA;
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

    const auto hash = std::hash<std::string>()(filepath.string());
    this->texture.insert(std::make_pair(hash, texture));

    LOG_I(filepath.string() << ": Succeed to create the file. (id=" << texture << ")");

    return texture;
}

GLuint TextureManager::LoadTexture(const std::filesystem::path& filepath, bool generate_mipmap)
{
    return CreateTextureFromFile(filepath, generate_mipmap);
}

void TextureManager::LoadTextures(const std::filesystem::path& directory, bool generate_mipmap)
{
    std::vector<std::filesystem::path> filepaths;
    auto func = [&filepaths](const std::filesystem::path& filepath)
    {
        if(std::filesystem::is_regular_file(filepath))
        {
            if(filepath.extension() == ".png" || filepath.extension() == ".exr")
            {
                filepaths.push_back(filepath.generic_string());
            }
        }
    };

    std::for_each(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator(), func);
    for(const auto& filepath : filepaths)
    {
        LoadTexture(filepath, generate_mipmap);
    }
}

void TextureManager::DeleteTexture(const std::filesystem::path& filepath)
{
    const auto hash = std::hash<std::string>()(filepath.string());
    DeleteTexture(hash);
}

void TextureManager::DeleteTexture(std::size_t hash)
{
    decltype(texture)::const_iterator it = texture.find(hash);
    if(it != texture.cend())
    {
        if(glIsTexture(it->second))
            glDeleteTextures(1, &it->second);
        texture.erase(it);
    }
}

void TextureManager::DeleteTextures()
{
    for(const auto& pair : texture)
    {
        if(glIsTexture(pair.second))
            glDeleteTextures(1, &pair.second);
    }
    texture.clear();
}

GLuint TextureManager::GetTexture(const std::filesystem::path& filepath) const
{
    const auto hash = std::hash<std::string>()(filepath.string());
    return GetTexture(hash);
}

GLuint TextureManager::GetTexture(std::size_t hash) const
{
    decltype(texture)::const_iterator it = texture.find(hash);
    return (it != texture.cend())? it->second : 0;
}

GLsizei TextureManager::CalcNumOfMipmapLevels(GLsizei width)
{
    return static_cast<GLsizei>(std::log2(static_cast<float>(width))) + 1;
}

GLsizei TextureManager::CalcNumOfMipmapLevels(GLsizei width, GLsizei height)
{
    return static_cast<GLsizei>(std::log2(static_cast<float>(std::max(width, height)))) + 1;
}