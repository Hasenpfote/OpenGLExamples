#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
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

    GLuint CreateTexture(const std::string& name, GLsizei levels, GLenum internal_format, GLsizei width, GLsizei height);
    GLuint CreateTexture(const std::string& name, GLenum internal_format, GLsizei width, GLsizei height);
    GLuint CreateTextureFromFile(const std::filesystem::path& filepath, bool generate_mipmap = true);

    GLuint LoadTexture(const std::filesystem::path& filepath, bool generate_mipmap = true);
    void LoadTextures(const std::filesystem::path& directory, bool generate_mipmap = true);

    void DeleteTexture(std::size_t hash);
    void DeleteTexture(const std::filesystem::path& filepath);
    void DeleteTextures();

    GLuint GetTexture(std::size_t hash) const;
    GLuint GetTexture(const std::filesystem::path& filepath) const;

    static GLsizei CalcNumOfMipmapLevels(GLsizei width);
    static GLsizei CalcNumOfMipmapLevels(GLsizei width, GLsizei height);

private:
    std::unordered_map<std::size_t, GLuint> texture;
};