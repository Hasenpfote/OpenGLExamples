#include <functional>
#include <hasenpfote/assert.h>
#include "logger.h"
#include "png_loader.h"
#include "exr_loader.h"
#include "texture.h"

TextureManager::~TextureManager()
{
    DeleteTextures();
}

GLuint TextureManager::CreateTexture(const std::string& name, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    HASENPFOTE_ASSERT(levels > 0);

    LOG_I("Creating texture file: " << name);
    auto texture = GetTexture(name);
    if(glIsTexture(texture)){
        LOG_E(name << ": Already exists.");
        return texture;
    }

    GLenum target = GL_TEXTURE_2D;

    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    glTexStorage2D(target, levels, internalformat, width, height);

    // Poor filtering
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels - 1);

    glBindTexture(GL_TEXTURE_2D, 0);

    const auto hash = std::hash<std::string>()(name);
    this->texture.insert(std::make_pair(hash, texture));

    LOG_I(name << ": Succeed to create the texture. (id=" << texture << ")");

    return texture;
}

GLuint TextureManager::CreateTexture(const std::string& name, GLenum internalformat, GLsizei width, GLsizei height)
{
    return CreateTexture(name, 1, internalformat, width, height);
}

GLuint TextureManager::CreateTextureFromPng(const std::filesystem::path& filepath, bool generates_mipmap)
{
    LOG_I("Creating texture file: " << filepath.string());
    auto texture = GetTexture(filepath.string());
    if(glIsTexture(texture)){
        LOG_E(filepath.string() << ": Already exists.");
        return texture;
    }

    PngLoader png;
    if(!png.Load(filepath)){
        LOG_E(filepath.string() << ": Failed to load the file.");
        return 0;
    }

    GLenum target = GL_TEXTURE_2D;
    GLenum format = (png.GetColorFormat() == PngLoader::ColorFormat::RGB)? GL_RGB : GL_RGBA;
    GLint alignment = (png.GetColorFormat() == PngLoader::ColorFormat::RGB)? 1 : 4;

    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glTexImage2D(target, 0, format, png.GetWidth(), png.GetHeight(), 0, format, GL_UNSIGNED_BYTE, png.GetData());

    GLsizei levels = 1;
    if(generates_mipmap){
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(target);
        levels = CalcNumOfMipmapLevels(png.GetWidth(), png.GetHeight());
    }
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels - 1);

    glBindTexture(GL_TEXTURE_2D, 0);

    const auto hash = std::hash<std::string>()(filepath.string());
    this->texture.insert(std::make_pair(hash, texture));

    LOG_I(filepath.string() << ": Succeed to create the file. (id=" << texture << ")");

    return texture;
}

GLuint TextureManager::CreateTextureFromExr(const std::filesystem::path& filepath, bool generates_mipmap)
{
    LOG_I("Creating texture file: " << filepath.string());
    auto texture = GetTexture(filepath.string());
    if(glIsTexture(texture))
    {
        LOG_E(filepath.string() << ": Already exists.");
        return texture;
    }

    ExrLoader exr;
    if(!exr.Load(filepath))
    {
        LOG_E(filepath.string() << ": Failed to load the file.");
        return 0;
    }

    GLenum target = GL_TEXTURE_2D;
    GLenum format;
    GLint internal_format;
    GLenum type;
    GLint alignment;

    if(exr.GetColorFormat() == ExrLoader::ColorFormat::RGB)
    {
        format = GL_RGB;
        if(exr.GetPixelType() == ExrLoader::PixelType::Half)
        {
            internal_format = GL_RGB16F;
            type = GL_HALF_FLOAT;
            alignment = 2;
        }
        else
        {
            internal_format = GL_RGB32F;
            type = GL_FLOAT;
            alignment = 4;
        }
    }
    else
    {
        format = GL_RGBA;
        if(exr.GetPixelType() == ExrLoader::PixelType::Half)
        {
            internal_format = GL_RGBA16F;
            type = GL_HALF_FLOAT;
            alignment = 8;
        }
        else
        {
            internal_format = GL_RGBA32F;
            type = GL_FLOAT;
            alignment = 4;
        }
    }

    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glTexImage2D(target, 0, internal_format, exr.GetWidth(), exr.GetHeight(), 0, format, type, exr.GetData());

    GLsizei levels = 1;
    if(generates_mipmap)
    {
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(target);
        levels = CalcNumOfMipmapLevels(exr.GetWidth(), exr.GetHeight());
    }
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels - 1);

    glBindTexture(GL_TEXTURE_2D, 0);

    const auto hash = std::hash<std::string>()(filepath.string());
    this->texture.insert(std::make_pair(hash, texture));

    LOG_I(filepath.string() << ": Succeed to create the file. (id=" << texture << ")");

    return texture;
}

GLuint TextureManager::LoadTexture(const std::filesystem::path& filepath, bool generates_mipmap)
{
    return CreateTextureFromPng(filepath, generates_mipmap);
}

void TextureManager::LoadTextures(const std::filesystem::path& directory, const std::filesystem::path& extension, bool generates_mipmap)
{
    std::vector<std::filesystem::path> filepaths;
    auto func = [&filepaths, &extension](const std::filesystem::path& filepath){
        if(std::filesystem::is_regular_file(filepath)){
            if(filepath.extension() == extension){
                filepaths.push_back(filepath.generic_string());
            }
        }
    };

    std::for_each(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator(), func);
    for(const auto& filepath : filepaths){
        LoadTexture(filepath, generates_mipmap);
    }
}

void TextureManager::LoadTextures(const std::filesystem::path& directory, bool generates_mipmap)
{
    std::vector<std::filesystem::path> filepaths;
    auto func = [&filepaths](const std::filesystem::path& filepath){
        if(std::filesystem::is_regular_file(filepath)){
            if(filepath.extension() == ".png" || filepath.extension() == ".exr"){
                filepaths.push_back(filepath.generic_string());
            }
        }
    };

    std::for_each(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator(), func);
    for(const auto& filepath : filepaths){
        if(filepath.extension() == ".png"){
            CreateTextureFromPng(filepath, generates_mipmap);
        }
        else
        if(filepath.extension() == ".exr"){
            CreateTextureFromExr(filepath, generates_mipmap);
        }
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
    if(it != texture.cend()){
        if(glIsTexture(it->second))
            glDeleteTextures(1, &it->second);
        texture.erase(it);
    }
}

void TextureManager::DeleteTextures()
{
    for(const auto& pair : texture){
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
    return std::log2(static_cast<float>(width)) + 1;
}

GLsizei TextureManager::CalcNumOfMipmapLevels(GLsizei width, GLsizei height)
{
    return std::log2(static_cast<float>(std::max(width, height))) + 1;
}