﻿#include <functional>
#include "logger.h"
#include "png_loader.h"
#include "texture.h"

namespace sys = std::tr2::sys;

TextureManager::~TextureManager()
{
    DeleteTextures();
}

GLuint TextureManager::LoadTexture(const sys::path& filepath, bool generates_mipmap)
{
    LOG_I("Loading texture file: " << filepath.string());
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
    GLenum format = (png.GetColorFormat() == PngLoader::ColorFormat::RGB)? GL_RGB : GL_RGBA;
    GLint alignment = (png.GetColorFormat() == PngLoader::ColorFormat::RGB)? 1 : 4;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glTexImage2D(GL_TEXTURE_2D, 0, format, png.GetWidth(), png.GetHeight(), 0, format, GL_UNSIGNED_BYTE, png.GetData());

    if(generates_mipmap){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    const auto hash = std::hash<std::string>()(filepath.string());
    this->texture.insert(std::make_pair(hash, texture));

    LOG_I(filepath.string() << ": Succeed to load the file. (id=" << texture << ")");

    return texture;
}

void TextureManager::LoadTextures(const sys::path& directory, const sys::path& extension, bool generates_mipmap)
{
    std::vector<sys::path> filepaths;
    auto func = [&filepaths, &extension](const sys::path& filepath){
        if(sys::is_regular_file(filepath)){
            if(filepath.extension() == extension){
                filepaths.push_back(filepath);
            }
        }
    };

    std::for_each(sys::directory_iterator(directory), sys::directory_iterator(), func);
    for(const auto& filepath : filepaths){
        LoadTexture(filepath, generates_mipmap);
    }
}

void TextureManager::DeleteTexture(const std::tr2::sys::path& filepath)
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

GLuint TextureManager::GetTexture(const std::tr2::sys::path& filepath) const
{
    const auto hash = std::hash<std::string>()(filepath.string());
    return GetTexture(hash);
}

GLuint TextureManager::GetTexture(std::size_t hash) const
{
    decltype(texture)::const_iterator it = texture.find(hash);
    return (it != texture.cend())? it->second : 0;
}