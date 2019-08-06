#pragma once
#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>
#include <GL/glew.h>
#include "../resource.h"

namespace common::render
{

class ShaderProgram final : public Resource<ShaderProgram>
{
    friend Resource<ShaderProgram>;
public:
    ShaderProgram(const std::string& source, GLenum type);
    ShaderProgram(const std::filesystem::path& filepath, GLenum type);
    ShaderProgram(const std::filesystem::path& filepath);
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator = (const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&&) = delete;
    ShaderProgram& operator = (ShaderProgram&&) = delete;

    GLuint GetProgram() const noexcept { return program_; }
    GLenum GetType() const noexcept { return type_; }

    //
    GLint GetUniformLocation(const std::string& name) const;
    GLuint GetUniformBlockIndex(const std::string& name) const;

    void SetUniform1f(const std::string& name, GLfloat v0);
    void SetUniform2f(const std::string& name, GLfloat v0, GLfloat v1);
    void SetUniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2);
    void SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

    void SetUniform1i(const std::string& name, GLint v0);
    void SetUniform2i(const std::string& name, GLint v0, GLint v1);
    void SetUniform3i(const std::string& name, GLint v0, GLint v1, GLint v2);
    void SetUniform4i(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3);

    void SetUniform1ui(const std::string& name, GLuint v0);
    void SetUniform2ui(const std::string& name, GLuint v0, GLuint v1);
    void SetUniform3ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2);
    void SetUniform4ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

    void SetUniform1fv(const std::string& name, GLsizei count, const GLfloat* value);
    void SetUniform2fv(const std::string& name, GLsizei count, const GLfloat* value);
    void SetUniform3fv(const std::string& name, GLsizei count, const GLfloat* value);
    void SetUniform4fv(const std::string& name, GLsizei count, const GLfloat* value);

    void SetUniform1iv(const std::string& name, GLsizei count, const GLint* value);
    void SetUniform2iv(const std::string& name, GLsizei count, const GLint* value);
    void SetUniform3iv(const std::string& name, GLsizei count, const GLint* value);
    void SetUniform4iv(const std::string& name, GLsizei count, const GLint* value);

    void SetUniform1uiv(const std::string& name, GLsizei count, const GLuint* value);
    void SetUniform2uiv(const std::string& name, GLsizei count, const GLuint* value);
    void SetUniform3uiv(const std::string& name, GLsizei count, const GLuint* value);
    void SetUniform4uiv(const std::string& name, GLsizei count, const GLuint* value);

    void SetUniformMatrix2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

    void SetUniformMatrix2x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix3x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix2x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix4x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix3x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix4x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

private:
    static const Resource<ShaderProgram>::string_set_t& allowed_extensions_impl();

private:
    GLuint program_;
    GLenum type_;
    mutable std::unordered_map<std::string, GLint> uniform_location_cache_;
    mutable std::unordered_map<std::string, GLuint> uniform_block_index_cache_;
};

class ShaderPipeline final
{
public:
    ShaderPipeline();
    ~ShaderPipeline();

    ShaderPipeline(const ShaderPipeline&) = delete;
    ShaderPipeline& operator = (const ShaderPipeline&) = delete;
    ShaderPipeline(ShaderPipeline&&) = delete;
    ShaderPipeline& operator = (ShaderPipeline&&) = delete;

    void Create();
    void Release();

    GLuint GetPipeline() const { return pipeline_; }

    void Bind();
    void Unbind();

    void SetShaderProgram(const ShaderProgram* sp);
    ShaderProgram* GetShaderProgram(GLenum type);
    const ShaderProgram* GetShaderProgram(GLenum type) const;
    void ResetShaderPrograms();

    void SetUniform1f(const std::string& name, GLfloat v0);
    void SetUniform2f(const std::string& name, GLfloat v0, GLfloat v1);
    void SetUniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2);
    void SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

    void SetUniform1i(const std::string& name, GLint v0);
    void SetUniform2i(const std::string& name, GLint v0, GLint v1);
    void SetUniform3i(const std::string& name, GLint v0, GLint v1, GLint v2);
    void SetUniform4i(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3);

    void SetUniform1ui(const std::string& name, GLuint v0);
    void SetUniform2ui(const std::string& name, GLuint v0, GLuint v1);
    void SetUniform3ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2);
    void SetUniform4ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

    void SetUniform1fv(const std::string& name, GLsizei count, const GLfloat* value);
    void SetUniform2fv(const std::string& name, GLsizei count, const GLfloat* value);
    void SetUniform3fv(const std::string& name, GLsizei count, const GLfloat* value);
    void SetUniform4fv(const std::string& name, GLsizei count, const GLfloat* value);

    void SetUniform1iv(const std::string& name, GLsizei count, const GLint* value);
    void SetUniform2iv(const std::string& name, GLsizei count, const GLint* value);
    void SetUniform3iv(const std::string& name, GLsizei count, const GLint* value);
    void SetUniform4iv(const std::string& name, GLsizei count, const GLint* value);

    void SetUniform1uiv(const std::string& name, GLsizei count, const GLuint* value);
    void SetUniform2uiv(const std::string& name, GLsizei count, const GLuint* value);
    void SetUniform3uiv(const std::string& name, GLsizei count, const GLuint* value);
    void SetUniform4uiv(const std::string& name, GLsizei count, const GLuint* value);

    void SetUniformMatrix2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

    void SetUniformMatrix2x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix3x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix2x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix4x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix3x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetUniformMatrix4x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

private:
    void Cache(const std::string& name);

private:
    GLuint pipeline_;
    std::unordered_map<GLenum, ShaderProgram*> shader_program_;
    std::unordered_multimap<std::string, ShaderProgram*> uniform_cache_;
};

}   // namespace common::render