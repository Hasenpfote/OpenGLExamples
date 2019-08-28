#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <GL/glew.h>

namespace common::render::shader
{

template<typename T>
class IUniform
{
public:
    IUniform() = default;
    virtual ~IUniform() = default;

    IUniform(const IUniform&) = delete;
    IUniform& operator = (const IUniform&) = delete;
    IUniform(IUniform&&) = delete;
    IUniform& operator = (IUniform&&) = delete;

    void Set1f(const std::string& name, GLfloat v0)
    {
        underlying().Set1f_impl(name, v0);
    }

    void Set2f(const std::string& name, GLfloat v0, GLfloat v1)
    {
        underlying().Set2f_impl(name, v0, v1);
    }

    void Set3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
    {
        underlying().Set3f_impl(name, v0, v1, v2);
    }

    void Set4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
    {
        underlying().Set4f_impl(name, v0, v1, v2, v3);
    }

    void Set1i(const std::string& name, GLint v0)
    {
        underlying().Set1i_impl(name, v0);
    }

    void Set2i(const std::string& name, GLint v0, GLint v1)
    {
        underlying().Set2i_impl(name, v0, v1);
    }

    void Set3i(const std::string& name, GLint v0, GLint v1, GLint v2)
    {
        underlying().Set3i_impl(name, v0, v1, v2);
    }

    void Set4i(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
    {
        underlying().Set4i_impl(name, v0, v1, v2, v3);
    }

    void Set1ui(const std::string& name, GLuint v0)
    {
        underlying().Set1ui_impl(name, v0);
    }

    void Set2ui(const std::string& name, GLuint v0, GLuint v1)
    {
        underlying().Set2ui_impl(name, v0, v1);
    }

    void Set3ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
    {
        underlying().Set3ui_impl(name, v0, v1, v2);
    }

    void Set4ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
    {
        underlying().Set4ui_impl(name, v0, v1, v2, v3);
    }

    void Set1fv(const std::string& name, GLsizei count, const GLfloat* value)
    {
        underlying().Set1fv_impl(name, count, value);
    }

    void Set2fv(const std::string& name, GLsizei count, const GLfloat* value)
    {
        underlying().Set2fv_impl(name, count, value);
    }

    void Set3fv(const std::string& name, GLsizei count, const GLfloat* value)
    {
        underlying().Set3fv_impl(name, count, value);
    }

    void Set4fv(const std::string& name, GLsizei count, const GLfloat* value)
    {
        underlying().Set4fv_impl(name, count, value);
    }

    void Set1iv(const std::string& name, GLsizei count, const GLint* value)
    {
        underlying().Set1iv_impl(name, count, value);
    }

    void Set2iv(const std::string& name, GLsizei count, const GLint* value)
    {
        underlying().Set2iv_impl(name, count, value);
    }

    void Set3iv(const std::string& name, GLsizei count, const GLint* value)
    {
        underlying().Set3iv_impl(name, count, value);
    }

    void Set4iv(const std::string& name, GLsizei count, const GLint* value)
    {
        underlying().Set4iv_impl(name, count, value);
    }

    void Set1uiv(const std::string& name, GLsizei count, const GLuint* value)
    {
        underlying().Set1uiv_impl(name, count, value);
    }

    void Set2uiv(const std::string& name, GLsizei count, const GLuint* value)
    {
        underlying().Set2uiv_impl(name, count, value);
    }

    void Set3uiv(const std::string& name, GLsizei count, const GLuint* value)
    {
        underlying().Set3uiv_impl(name, count, value);
    }

    void Set4uiv(const std::string& name, GLsizei count, const GLuint* value)
    {
        underlying().Set4uiv_impl(name, count, value);
    }

    void SetMatrix2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix2fv_impl(name, count, transpose, value);
    }

    void SetMatrix3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix3fv_impl(name, count, transpose, value);
    }

    void SetMatrix4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix4fv_impl(name, count, transpose, value);
    }

    void SetMatrix2x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix2x3fv_impl(name, count, transpose, value);
    }

    void SetMatrix3x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix3x2fv_impl(name, count, transpose, value);
    }

    void SetMatrix2x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix2x4fv_impl(name, count, transpose, value);
    }

    void SetMatrix4x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix4x2fv_impl(name, count, transpose, value);
    }

    void SetMatrix3x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix3x4fv_impl(name, count, transpose, value);
    }

    void SetMatrix4x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
    {
        underlying().SetMatrix4x3fv_impl(name, count, transpose, value);
    }

protected:
    void Set1f_impl(const std::string& name, GLfloat v0);
    void Set2f_impl(const std::string& name, GLfloat v0, GLfloat v1);
    void Set3f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2);
    void Set4f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

    void Set1i_impl(const std::string& name, GLint v0);
    void Set2i_impl(const std::string& name, GLint v0, GLint v1);
    void Set3i_impl(const std::string& name, GLint v0, GLint v1, GLint v2);
    void Set4i_impl(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3);

    void Set1ui_impl(const std::string& name, GLuint v0);
    void Set2ui_impl(const std::string& name, GLuint v0, GLuint v1);
    void Set3ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2);
    void Set4ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

    void Set1fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set2fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set3fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set4fv_impl(const std::string& name, GLsizei count, const GLfloat* value);

    void Set1iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set2iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set3iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set4iv_impl(const std::string& name, GLsizei count, const GLint* value);

    void Set1uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set2uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set3uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set4uiv_impl(const std::string& name, GLsizei count, const GLuint* value);

    void SetMatrix2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

    void SetMatrix2x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix2x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

private:
    T& underlying() noexcept { return static_cast<T&>(*this); }
    const T& underlying() const noexcept { return static_cast<const T&>(*this); }
};


class Uniform final : public IUniform<Uniform>
{
    friend IUniform<Uniform>;
public:
    explicit Uniform(GLuint program)
        : program_(program)
    {}

    ~Uniform() = default;

    Uniform(const Uniform&) = delete;
    Uniform& operator = (const Uniform&) = delete;
    Uniform(Uniform&&) = delete;
    Uniform& operator = (Uniform&&) = delete;

    GLint Uniform::GetLocation(const std::string& name) const;
    GLuint Uniform::GetBlockIndex(const std::string& name) const;

private:

    void Set1f_impl(const std::string& name, GLfloat v0);
    void Set2f_impl(const std::string& name, GLfloat v0, GLfloat v1);
    void Set3f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2);
    void Set4f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

    void Set1i_impl(const std::string& name, GLint v0);
    void Set2i_impl(const std::string& name, GLint v0, GLint v1);
    void Set3i_impl(const std::string& name, GLint v0, GLint v1, GLint v2);
    void Set4i_impl(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3);

    void Set1ui_impl(const std::string& name, GLuint v0);
    void Set2ui_impl(const std::string& name, GLuint v0, GLuint v1);
    void Set3ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2);
    void Set4ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

    void Set1fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set2fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set3fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set4fv_impl(const std::string& name, GLsizei count, const GLfloat* value);

    void Set1iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set2iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set3iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set4iv_impl(const std::string& name, GLsizei count, const GLint* value);

    void Set1uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set2uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set3uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set4uiv_impl(const std::string& name, GLsizei count, const GLuint* value);

    void SetMatrix2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

    void SetMatrix2x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix2x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

private:
    GLuint program_;
    mutable std::unordered_map<std::string, GLint> location_cache_;
    mutable std::unordered_map<std::string, GLuint> block_index_cache_;
};


class PipelineUniform final : public IUniform<PipelineUniform>
{
    friend IUniform<PipelineUniform>;
public:
    using UniformPtrSet = std::unordered_set<Uniform*>;

public:
    explicit PipelineUniform(const UniformPtrSet& ups);

    ~PipelineUniform() = default;

    PipelineUniform(const PipelineUniform&) = delete;
    PipelineUniform& operator = (const PipelineUniform&) = delete;
    PipelineUniform(PipelineUniform&&) = delete;
    PipelineUniform& operator = (PipelineUniform&&) = delete;

private:
    void Cache(const std::string& name);

    void Set1f_impl(const std::string& name, GLfloat v0);
    void Set2f_impl(const std::string& name, GLfloat v0, GLfloat v1);
    void Set3f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2);
    void Set4f_impl(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

    void Set1i_impl(const std::string& name, GLint v0);
    void Set2i_impl(const std::string& name, GLint v0, GLint v1);
    void Set3i_impl(const std::string& name, GLint v0, GLint v1, GLint v2);
    void Set4i_impl(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3);

    void Set1ui_impl(const std::string& name, GLuint v0);
    void Set2ui_impl(const std::string& name, GLuint v0, GLuint v1);
    void Set3ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2);
    void Set4ui_impl(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

    void Set1fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set2fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set3fv_impl(const std::string& name, GLsizei count, const GLfloat* value);
    void Set4fv_impl(const std::string& name, GLsizei count, const GLfloat* value);

    void Set1iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set2iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set3iv_impl(const std::string& name, GLsizei count, const GLint* value);
    void Set4iv_impl(const std::string& name, GLsizei count, const GLint* value);

    void Set1uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set2uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set3uiv_impl(const std::string& name, GLsizei count, const GLuint* value);
    void Set4uiv_impl(const std::string& name, GLsizei count, const GLuint* value);

    void SetMatrix2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

    void SetMatrix2x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix2x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4x2fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix3x4fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);
    void SetMatrix4x3fv_impl(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value);

private:
    UniformPtrSet ups_;
    std::unordered_multimap<std::string, Uniform*> cache_;
};

}   // namespace common::render::shader