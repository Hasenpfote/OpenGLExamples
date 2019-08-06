#pragma once
#include <filesystem>
#include <GL/glew.h>
#include "../resource.h"

namespace common::render
{

class Texture final : public Resource<Texture>
{
    friend Resource<Texture>;
public:
    Texture(GLsizei levels, GLenum internal_format, GLsizei width, GLsizei height);
    Texture(GLenum internal_format, GLsizei width, GLsizei height);
    Texture(const std::filesystem::path& filepath, bool generate_mipmap = true);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator = (const Texture&) = delete;
    Texture(Texture&&) = delete;
    Texture& operator = (Texture&&) = delete;

    GLuint GetTexture() const noexcept { return texture_; };

    static GLsizei CalcNumOfMipmapLevels(GLsizei width);
    static GLsizei CalcNumOfMipmapLevels(GLsizei width, GLsizei height);

private:
    static const string_set_t& allowed_extensions_impl()
    {
        static string_set_t ss({ ".png", ".exr" });
        return ss;
    }

private:
    GLuint texture_;
};

}   // namespace common::render