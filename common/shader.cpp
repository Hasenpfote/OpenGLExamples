#include <stdexcept>
#include <fstream>
#include <functional>
#include <hasenpfote/assert.h>
#include "logger.h"
#include "shader.h"

namespace
{

GLuint create_shader_program(const std::string& source, GLenum type)
{
    auto s = source.c_str();
    GLuint program = glCreateShaderProgramv(type, 1, &s);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        if(log_length > 0)
        {
            std::vector<GLchar> log(log_length);
            GLsizei length = 0;
            glGetProgramInfoLog(program, log_length, &length, log.data());
            std::istringstream iss(std::string(log.data()));
            std::string field;
            while(std::getline(iss, field, '\n'))
            {
                LOG_E("ProgramInfoLog:" << field);
            }
        }
        glDeleteProgram(program);

        throw std::runtime_error("");
    }
    return program;
}

GLenum file_extension_to_shader_type(const std::filesystem::path& extension)
{
    const auto ext = extension.string();
    if(ext == ".vs")
        return GL_VERTEX_SHADER;
    if(ext == ".tcs")
        return GL_TESS_CONTROL_SHADER;
    if(ext == ".tes")
        return GL_TESS_EVALUATION_SHADER;
    if(ext == ".gs")
        return GL_GEOMETRY_SHADER;
    if(ext == ".fs")
        return GL_FRAGMENT_SHADER;
    return 0;
}

GLbitfield shader_type_to_stage(GLenum type)
{
    if(type == GL_VERTEX_SHADER)
        return GL_VERTEX_SHADER_BIT;
    if(type == GL_TESS_CONTROL_SHADER)
        return GL_TESS_CONTROL_SHADER_BIT;
    if(type == GL_TESS_EVALUATION_SHADER)
        return GL_TESS_EVALUATION_SHADER_BIT;
    if(type == GL_GEOMETRY_SHADER)
        return GL_GEOMETRY_SHADER_BIT;
    if(type == GL_FRAGMENT_SHADER)
        return GL_FRAGMENT_SHADER_BIT;
    return 0;
}

}

namespace common
{

ShaderProgram::ShaderProgram(const std::string& source, GLenum type)
    : program_(0), type_(0)
{
    program_ = create_shader_program(source, type);
    type_ = type;
}

ShaderProgram::ShaderProgram(const std::filesystem::path& filepath, GLenum type)
    : program_(0), type_(0)
{
    std::ifstream ifs(filepath.string(), std::ios::in | std::ios::binary);
    if(ifs.fail())
    {
        LOG_E("Could not read shader: " << filepath.filename().string());
        throw std::runtime_error("");
    }
    std::istreambuf_iterator<GLchar> it(ifs);
    std::istreambuf_iterator<GLchar> last;
    std::string source(it, last);

    program_ = create_shader_program(source, type);
    type_ = type;
}

ShaderProgram::ShaderProgram(const std::filesystem::path& filepath)
    : ShaderProgram(filepath, file_extension_to_shader_type(filepath.extension()))
{
}

ShaderProgram::~ShaderProgram()
{
    if(glIsProgram(program_))
        glDeleteProgram(program_);
}

GLint ShaderProgram::GetUniformLocation(const std::string& name) const
{
    HASENPFOTE_ASSERT(glIsProgram(program_));

    decltype(uniform_location_cache_)::const_iterator it = uniform_location_cache_.find(name);
    if(it != uniform_location_cache_.cend())
    {
        return it->second;
    }
    auto loc = glGetUniformLocation(program_, name.c_str());
    if(loc >= 0)
    {
        uniform_location_cache_.insert(std::make_pair(name, loc));  // caching
    }
    return loc;
}

GLuint ShaderProgram::GetUniformBlockIndex(const std::string& name) const
{
    HASENPFOTE_ASSERT(glIsProgram(program_));

    decltype(uniform_block_index_cache_)::const_iterator it = uniform_block_index_cache_.find(name);
    if(it != uniform_block_index_cache_.cend())
    {
        return it->second;
    }
    auto index = glGetUniformBlockIndex(program_, name.c_str());
    if(index != GL_INVALID_INDEX)
    {
        uniform_block_index_cache_.insert(std::make_pair(name, index));  // caching
    }
    return index;
}

void ShaderProgram::SetUniform1f(const std::string& name, GLfloat v0)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1f(program_, loc, v0);
}

void ShaderProgram::SetUniform2f(const std::string& name, GLfloat v0, GLfloat v1)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2f(program_, loc, v0, v1);
}

void ShaderProgram::SetUniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3f(program_, loc, v0, v1, v2);
}

void ShaderProgram::SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4f(program_, loc, v0, v1, v2, v3);
}

void ShaderProgram::SetUniform1i(const std::string& name, GLint v0)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1i(program_, loc, v0);
}

void ShaderProgram::SetUniform2i(const std::string& name, GLint v0, GLint v1)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2i(program_, loc, v0, v1);
}

void ShaderProgram::SetUniform3i(const std::string& name, GLint v0, GLint v1, GLint v2)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3i(program_, loc, v0, v1, v2);
}

void ShaderProgram::SetUniform4i(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4i(program_, loc, v0, v1, v2, v3);
}

void ShaderProgram::SetUniform1ui(const std::string& name, GLuint v0)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1ui(program_, loc, v0);
}

void ShaderProgram::SetUniform2ui(const std::string& name, GLuint v0, GLuint v1)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2ui(program_, loc, v0, v1);
}

void ShaderProgram::SetUniform3ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3ui(program_, loc, v0, v1, v2);
}

void ShaderProgram::SetUniform4ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4ui(program_, loc, v0, v1, v2, v3);
}

void ShaderProgram::SetUniform1fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1fv(program_, loc, count, value);
}

void ShaderProgram::SetUniform2fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2fv(program_, loc, count, value);
}

void ShaderProgram::SetUniform3fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3fv(program_, loc, count, value);
}

void ShaderProgram::SetUniform4fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4fv(program_, loc, count, value);
}

void ShaderProgram::SetUniform1iv(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1iv(program_, loc, count, value);
}

void ShaderProgram::SetUniform2iv(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2iv(program_, loc, count, value);
}

void ShaderProgram::SetUniform3iv(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3iv(program_, loc, count, value);
}

void ShaderProgram::SetUniform4iv(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4iv(program_, loc, count, value);
}

void ShaderProgram::SetUniform1uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1uiv(program_, loc, count, value);
}

void ShaderProgram::SetUniform2uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2uiv(program_, loc, count, value);
}

void ShaderProgram::SetUniform3uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3uiv(program_, loc, count, value);
}

void ShaderProgram::SetUniform4uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4uiv(program_, loc, count, value);
}

void ShaderProgram::SetUniformMatrix2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2fv(program_, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3fv(program_, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4fv(program_, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix2x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2x3fv(program_, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix3x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3x2fv(program_, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix2x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2x4fv(program_, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix4x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4x2fv(program_, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix3x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3x4fv(program_, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix4x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4x3fv(program_, loc, count, transpose, value);
}

const common::Resource<ShaderProgram>::string_set_t& ShaderProgram::allowed_extensions_impl()
{
    static string_set_t ss({ ".vs", ".tcs", ".tes", ".gs", ".fs" });
    return ss;
}


ShaderPipeline::ShaderPipeline()
    : pipeline_(0)
{
}

ShaderPipeline::~ShaderPipeline()
{
    if(glIsProgramPipeline(pipeline_))
        glDeleteProgramPipelines(1, &pipeline_);
}

void ShaderPipeline::Create()
{
    if(!glIsProgramPipeline(pipeline_))
        glGenProgramPipelines(1, &pipeline_);
}

void ShaderPipeline::Release()
{
    if(glIsProgramPipeline(pipeline_))
    {
        glDeleteProgramPipelines(1, &pipeline_);
    }
    pipeline_ = 0;
    shader_program_.clear();
    uniform_cache_.clear();
}

void ShaderPipeline::Bind()
{
    HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline_));
    glUseProgram(0);
    glBindProgramPipeline(pipeline_);
}

void ShaderPipeline::Unbind()
{
    glBindProgramPipeline(0);
}

void ShaderPipeline::SetShaderProgram(const ShaderProgram* sp)
{
    HASENPFOTE_ASSERT(sp != nullptr);

    auto type = sp->GetType();

    GLbitfield stage = shader_type_to_stage(type);
    HASENPFOTE_ASSERT_MSG(stage > 0, "Unknown type");

    glUseProgramStages(pipeline_, stage, sp->GetProgram());
    HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline_));

    decltype(shader_program_)::const_iterator it = shader_program_.find(type);
    if(it != shader_program_.cend())
    {
        shader_program_.erase(it);
        uniform_cache_.clear();
    }
    shader_program_.insert(std::make_pair(type, const_cast<ShaderProgram*>(sp)));
}

ShaderProgram* ShaderPipeline::GetShaderProgram(GLenum type)
{
    decltype(shader_program_)::const_iterator it = shader_program_.find(type);
    return (it != shader_program_.cend())? it->second : nullptr;
}

const ShaderProgram* ShaderPipeline::GetShaderProgram(GLenum type) const
{
    decltype(shader_program_)::const_iterator it = shader_program_.find(type);
    return (it != shader_program_.cend())? it->second : nullptr;
}

void ShaderPipeline::ResetShaderPrograms()
{
    HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline_));
    glUseProgramStages(pipeline_, GL_ALL_SHADER_BITS, 0);
    shader_program_.clear();
    uniform_cache_.clear();
}

void ShaderPipeline::SetUniform1f(const std::string& name, GLfloat v0)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform1f(name, v0);
    }
}

void ShaderPipeline::SetUniform2f(const std::string& name, GLfloat v0, GLfloat v1)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform2f(name, v0, v1);
    }
}

void ShaderPipeline::SetUniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform3f(name, v0, v1, v2);
    }
}

void ShaderPipeline::SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform4f(name, v0, v1, v2, v3);
    }
}

void ShaderPipeline::SetUniform1i(const std::string& name, GLint v0)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform1i(name, v0);
    }
}

void ShaderPipeline::SetUniform2i(const std::string& name, GLint v0, GLint v1)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform2i(name, v0, v1);
    }
}

void ShaderPipeline::SetUniform3i(const std::string& name, GLint v0, GLint v1, GLint v2)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform3i(name, v0, v1, v2);
    }
}

void ShaderPipeline::SetUniform4i(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform4i(name, v0, v1, v2, v3);
    }
}

void ShaderPipeline::SetUniform1ui(const std::string& name, GLuint v0)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform1ui(name, v0);
    }
}

void ShaderPipeline::SetUniform2ui(const std::string& name, GLuint v0, GLuint v1)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform2ui(name, v0, v1);
    }
}

void ShaderPipeline::SetUniform3ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform3ui(name, v0, v1, v2);
    }
}

void ShaderPipeline::SetUniform4ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform4ui(name, v0, v1, v2, v3);
    }
}

void ShaderPipeline::SetUniform1fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform1fv(name, count, value);
    }
}

void ShaderPipeline::SetUniform2fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform2fv(name, count, value);
    }
}

void ShaderPipeline::SetUniform3fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform3fv(name, count, value);
    }
}

void ShaderPipeline::SetUniform4fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform4fv(name, count, value);
    }
}

void ShaderPipeline::SetUniform1iv(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform1iv(name, count, value);
    }
}

void ShaderPipeline::SetUniform2iv(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform2iv(name, count, value);
    }
}

void ShaderPipeline::SetUniform3iv(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform3iv(name, count, value);
    }
}

void ShaderPipeline::SetUniform4iv(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform4iv(name, count, value);
    }
}

void ShaderPipeline::SetUniform1uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform1uiv(name, count, value);
    }
}

void ShaderPipeline::SetUniform2uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform2uiv(name, count, value);
    }
}

void ShaderPipeline::SetUniform3uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform3uiv(name, count, value);
    }
}

void ShaderPipeline::SetUniform4uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniform4uiv(name, count, value);
    }
}

void ShaderPipeline::SetUniformMatrix2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix2fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix3fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix4fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix2x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix2x3fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix3x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix3x2fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix2x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix2x4fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix4x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix4x2fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix3x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix3x4fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix4x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache_.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++)
    {
        it->second->SetUniformMatrix4x3fv(name, count, transpose, value);
    }
}

void ShaderPipeline::Cache(const std::string& name)
{
    if(uniform_cache_.find(name) != uniform_cache_.cend())
        return;

    for(const auto& pair : shader_program_)
    {
        if(pair.second->GetUniformLocation(name) >= 0)
        {
            uniform_cache_.insert(std::make_pair(name, pair.second));
        }
    }
    HASENPFOTE_ASSERT_MSG(uniform_cache_.find(name) != uniform_cache_.cend(), "Could not find uniform variable `" << name << "` in shaders.");
}

}   // namespace common