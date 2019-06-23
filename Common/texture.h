#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>   // std::tr2::sys::path etc.
#include <GL/glew.h>

class TextureManager final
{
public:
    TextureManager() = default;
    ~TextureManager();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator = (const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator = (TextureManager&&) = delete;

    GLuint LoadTexture(const std::filesystem::path& filepath, bool generates_mipmap = true);
    void LoadTextures(const std::filesystem::path& directory, const std::filesystem::path& extension, bool generates_mipmap = true);

    void DeleteTexture(std::size_t hash);
    void DeleteTexture(const std::filesystem::path& filepath);
    void DeleteTextures();

    GLuint GetTexture(std::size_t hash) const;
    GLuint GetTexture(const std::filesystem::path& filepath) const;

private:
    std::unordered_map<std::size_t, GLuint> texture;
};