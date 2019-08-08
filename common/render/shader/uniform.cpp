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

void Uniform::Set1f_impl(const std::string& name, GLfloat v0)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1f(program_, loc, v0);
}

void Uniform::Set2f_impl(const std::string& name, GLfloat v0, GLfloat v1)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2f(program_, loc, v0, v1);
}

void Uniform::Set3f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3f(program_, loc, v0, v1, v2);
}

void Uniform::Set4f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4f(program_, loc, v0, v1, v2, v3);
}

void Uniform::Set1i_impl(const std::string& name, GLint v0)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1i(program_, loc, v0);
}

void Uniform::Set2i_impl(const std::string& name, GLint v0, GLint v1)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2i(program_, loc, v0, v1);
}

void Uniform::Set3i_impl(const std::string& name, GLint v0, GLint v1, GLint v2)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3i(program_, loc, v0, v1, v2);
}

void Uniform::Set4i_impl(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4i(program_, loc, v0, v1, v2, v3);
}

void Uniform::Set1ui_impl(const std::string& name, GLuint v0)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1ui(program_, loc, v0);
}

void Uniform::Set2ui_impl(const std::string& name, GLuint v0, GLuint v1)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2ui(program_, loc, v0, v1);
}

void Uniform::Set3ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3ui(program_, loc, v0, v1, v2);
}

void Uniform::Set4ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4ui(program_, loc, v0, v1, v2, v3);
}

void Uniform::Set1fv_impl(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1fv(program_, loc, count, value);
}

void Uniform::Set2fv_impl(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2fv(program_, loc, count, value);
}

void Uniform::Set3fv_impl(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3fv(program_, loc, count, value);
}

void Uniform::Set4fv_impl(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4fv(program_, loc, count, value);
}

void Uniform::Set1iv_impl(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1iv(program_, loc, count, value);
}

void Uniform::Set2iv_impl(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2iv(program_, loc, count, value);
}

void Uniform::Set3iv_impl(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3iv(program_, loc, count, value);
}

void Uniform::Set4iv_impl(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4iv(program_, loc, count, value);
}

void Uniform::Set1uiv_impl(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1uiv(program_, loc, count, value);
}

void Uniform::Set2uiv_impl(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2uiv(program_, loc, count, value);
}

void Uniform::Set3uiv_impl(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3uiv(program_, loc, count, value);
}

void Uniform::Set4uiv_impl(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4uiv(program_, loc, count, value);
}

void Uniform::SetMatrix2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2fv(program_, loc, count, transpose, value);
}

void Uniform::SetMatrix3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3fv(program_, loc, count, transpose, value);
}

void Uniform::SetMatrix4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4fv(program_, loc, count, transpose, value);
}

void Uniform::SetMatrix2x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2x3fv(program_, loc, count, transpose, value);
}

void Uniform::SetMatrix3x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3x2fv(program_, loc, count, transpose, value);
}

void Uniform::SetMatrix2x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2x4fv(program_, loc, count, transpose, value);
}

void Uniform::SetMatrix4x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4x2fv(program_, loc, count, transpose, value);
}

void Uniform::SetMatrix3x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3x4fv(program_, loc, count, transpose, value);
}

void Uniform::SetMatrix4x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4x3fv(program_, loc, count, transpose, value);
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

void PipelineUniform::Set1f_impl(const std::string& name, GLfloat v0)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set1f(name, v0); });
}

void PipelineUniform::Set2f_impl(const std::string& name, GLfloat v0, GLfloat v1)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set2f(name, v0, v1); });
}

void PipelineUniform::Set3f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set3f(name, v0, v1, v2); });
}

void PipelineUniform::Set4f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set4f(name, v0, v1, v2, v3); });
}

void PipelineUniform::Set1i_impl(const std::string& name, GLint v0)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set1i(name, v0); });
}

void PipelineUniform::Set2i_impl(const std::string& name, GLint v0, GLint v1)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set2i(name, v0, v1); });
}

void PipelineUniform::Set3i_impl(const std::string& name, GLint v0, GLint v1, GLint v2)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set3i(name, v0, v1, v2); });
}

void PipelineUniform::Set4i_impl(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set4i(name, v0, v1, v2, v3); });
}

void PipelineUniform::Set1ui_impl(const std::string& name, GLuint v0)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set1ui(name, v0); });
}

void PipelineUniform::Set2ui_impl(const std::string& name, GLuint v0, GLuint v1)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set2ui(name, v0, v1); });
}

void PipelineUniform::Set3ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set3ui(name, v0, v1, v2); });
}

void PipelineUniform::Set4ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set4ui(name, v0, v1, v2, v3); });
}

void PipelineUniform::Set1fv_impl(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set1fv(name, count, value); });
}

void PipelineUniform::Set2fv_impl(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set2fv(name, count, value); });
}

void PipelineUniform::Set3fv_impl(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set3fv(name, count, value); });
}

void PipelineUniform::Set4fv_impl(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set4fv(name, count, value); });
}

void PipelineUniform::Set1iv_impl(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set1iv(name, count, value); });
}

void PipelineUniform::Set2iv_impl(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set2iv(name, count, value); });
}

void PipelineUniform::Set3iv_impl(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set3iv(name, count, value); });
}

void PipelineUniform::Set4iv_impl(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set4iv(name, count, value); });
}

void PipelineUniform::Set1uiv_impl(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set1uiv(name, count, value); });
}

void PipelineUniform::Set2uiv_impl(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set2uiv(name, count, value); });
}

void PipelineUniform::Set3uiv_impl(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set3uiv(name, count, value); });
}

void PipelineUniform::Set4uiv_impl(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->Set4uiv(name, count, value); });
}

void PipelineUniform::SetMatrix2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix2fv(name, count, transpose, value); });
}

void PipelineUniform::SetMatrix3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix3fv(name, count, transpose, value); });
}

void PipelineUniform::SetMatrix4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix4fv(name, count, transpose, value); });
}

void PipelineUniform::SetMatrix2x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix2x3fv(name, count, transpose, value); });
}

void PipelineUniform::SetMatrix3x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix3x2fv(name, count, transpose, value); });
}

void PipelineUniform::SetMatrix2x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix2x4fv(name, count, transpose, value); });
}

void PipelineUniform::SetMatrix4x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix4x2fv(name, count, transpose, value); });
}

void PipelineUniform::SetMatrix3x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix3x4fv(name, count, transpose, value); });
}

void PipelineUniform::SetMatrix4x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto range = cache_.equal_range(name);
    std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x) { x.second->SetMatrix4x3fv(name, count, transpose, value); });
}

}   // namespace common::render::shader