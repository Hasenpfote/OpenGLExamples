#include <algorithm>
#include <hasenpfote/assert.h>
#include "../../logger.h"
#include "uniform.h"

namespace common::render::shader
{

GLint Uniform::GetLocation(const std::string& name) const
{
    HASENPFOTE_ASSERT(glIsProgram(program_));

    decltype(location_cache_)::const_iterator it = location_cache_.find(name);
    if(it != location_cache_.cend())
    {
        return it->second;
    }

    auto location = glGetUniformLocation(program_, name.c_str());
    if(location >= 0)
    {
        location_cache_.emplace(name, location); // caching
    }
    return location;
}

GLuint Uniform::GetBlockIndex(const std::string& name) const
{
    HASENPFOTE_ASSERT(glIsProgram(program_));

    decltype(block_index_cache_)::const_iterator it = block_index_cache_.find(name);
    if(it != block_index_cache_.cend())
    {
        return it->second;
    }

    auto index = glGetUniformBlockIndex(program_, name.c_str());
    if(index != GL_INVALID_INDEX)
    {
        block_index_cache_.emplace(name, index); // caching
    }
    //LOG_W("Could not find uniform block index `" << name << "` in shader.");
    return index;
}

template<>
void Uniform::set_impl<GLfloat>(const std::string& name, GLfloat v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1f(program_, loc, v);
}

template<>
void Uniform::set_impl<glm::vec1>(const std::string& name, const glm::vec1& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1f(program_, loc, v.x);
}

template<>
void Uniform::set_impl<glm::vec2>(const std::string& name, const glm::vec2& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2f(program_, loc, v.x, v.y);
}

template<>
void Uniform::set_impl<glm::vec3>(const std::string& name, const glm::vec3& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3f(program_, loc, v.x, v.y, v.z);
}

template<>
void Uniform::set_impl<glm::vec4>(const std::string& name, const glm::vec4& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4f(program_, loc, v.x, v.y, v.z, v.w);
}

template<>
void Uniform::set_impl<GLint>(const std::string& name, GLint v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1i(program_, loc, v);
}

template<>
void Uniform::set_impl<glm::ivec1>(const std::string& name, const glm::ivec1& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1i(program_, loc, v.x);
}

template<>
void Uniform::set_impl<glm::ivec2>(const std::string& name, const glm::ivec2& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2i(program_, loc, v.x, v.y);
}

template<>
void Uniform::set_impl<glm::ivec3>(const std::string& name, const glm::ivec3& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3i(program_, loc, v.x, v.y, v.z);
}

template<>
void Uniform::set_impl<glm::ivec4>(const std::string& name, const glm::ivec4& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4i(program_, loc, v.x, v.y, v.z, v.w);
}

template<>
void Uniform::set_impl<GLuint>(const std::string& name, GLuint v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1ui(program_, loc, v);
}

template<>
void Uniform::set_impl<glm::uvec1>(const std::string& name, const glm::uvec1& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1ui(program_, loc, v.x);
}

template<>
void Uniform::set_impl<glm::uvec2>(const std::string& name, const glm::uvec2& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2ui(program_, loc, v.x, v.y);
}

template<>
void Uniform::set_impl<glm::uvec3>(const std::string& name, const glm::uvec3& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3ui(program_, loc, v.x, v.y, v.z);
}

template<>
void Uniform::set_impl<glm::uvec4>(const std::string& name, const glm::uvec4& v)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4ui(program_, loc, v.x, v.y, v.z, v.w);
}

template<>
void Uniform::set_impl<glm::vec1>(const std::string& name, const glm::vec1 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1fv(program_, loc, size, const_cast<GLfloat*>(&a[0].x));
}

template<>
void Uniform::set_impl<glm::vec2>(const std::string& name, const glm::vec2 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2fv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::vec3>(const std::string& name, const glm::vec3 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3fv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::vec4>(const std::string& name, const glm::vec4 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4fv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::ivec1>(const std::string& name, const glm::ivec1 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1iv(program_, loc, size, const_cast<GLint*>(&a[0].x));
}

template<>
void Uniform::set_impl<glm::ivec2>(const std::string& name, const glm::ivec2 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2iv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::ivec3>(const std::string& name, const glm::ivec3 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3iv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::ivec4>(const std::string& name, const glm::ivec4 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4iv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::uvec1>(const std::string& name, const glm::uvec1 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1uiv(program_, loc, size, const_cast<GLuint*>(&a[0].x));
}

template<>
void Uniform::set_impl<glm::uvec2>(const std::string& name, const glm::uvec2 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2uiv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::uvec3>(const std::string& name, const glm::uvec3 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3uiv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::uvec4>(const std::string& name, const glm::uvec4 a[], std::size_t size)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4uiv(program_, loc, size, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat2>(const std::string& name, const glm::mat2 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat3>(const std::string& name, const glm::mat3 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat4>(const std::string& name, const glm::mat4 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat2x3>(const std::string& name, const glm::mat2x3 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2x3fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat3x2>(const std::string& name, const glm::mat3x2 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3x2fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat2x4>(const std::string& name, const glm::mat2x4 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2x4fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat4x2>(const std::string& name, const glm::mat4x2 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4x2fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat3x4>(const std::string& name, const glm::mat3x4 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3x4fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

template<>
void Uniform::set_impl<glm::mat4x3>(const std::string& name, const glm::mat4x3 a[], std::size_t size, bool transpose)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4x3fv(program_, loc, size, transpose, glm::value_ptr(a[0]));
}

PipelineUniform::PipelineUniform(const UniformPtrSet& ups)
    : ups_(ups)
{
}

void PipelineUniform::Cache(const std::string& name)
{
    if(cache_.find(name) != cache_.end())
        return;

    for(auto elem : ups_)
    {
        if(elem->GetLocation(name) >= 0)
        {
            cache_.emplace(name, elem);
        }
    }
    HASENPFOTE_ASSERT_MSG(cache_.find(name) != cache_.end(), "Could not find uniform variable `" << name << "` in shaders.");
}

}   // namespace common::render::shader