#pragma once
#include <type_traits>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace common::render::shader
{

template<typename T>
class IUniform
{
public:
    void Set(const std::string& name, GLfloat v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::vec1& v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::vec2& v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::vec3& v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::vec4& v){ underlying().set_impl(name, v); }

    void Set(const std::string& name, GLint v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::ivec1& v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::ivec2& v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::ivec3& v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::ivec4& v){ underlying().set_impl(name, v); }

    void Set(const std::string& name, GLuint v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::uvec2& v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::uvec3& v){ underlying().set_impl(name, v); }
    void Set(const std::string& name, const glm::uvec4& v){ underlying().set_impl(name, v); }

    void Set(const std::string& name, const glm::vec1 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::vec2 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::vec3 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::vec4 a[], std::size_t size){ underlying().set_impl(name, a, size); }

    void Set(const std::string& name, const glm::ivec1 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::ivec2 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::ivec3 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::ivec4 a[], std::size_t size){ underlying().set_impl(name, a, size); }

    void Set(const std::string& name, const glm::uvec1 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::uvec2 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::uvec3 a[], std::size_t size){ underlying().set_impl(name, a, size); }
    void Set(const std::string& name, const glm::uvec4 a[], std::size_t size){ underlying().set_impl(name, a, size); }

    void Set(const std::string& name, const glm::mat2 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }
    void Set(const std::string& name, const glm::mat3 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }
    void Set(const std::string& name, const glm::mat4 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }

    void Set(const std::string& name, const glm::mat2x3 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }
    void Set(const std::string& name, const glm::mat3x2 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }
    void Set(const std::string& name, const glm::mat2x4 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }
    void Set(const std::string& name, const glm::mat4x2 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }
    void Set(const std::string& name, const glm::mat3x4 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }
    void Set(const std::string& name, const glm::mat4x3 a[], std::size_t size, bool transpose = false){ underlying().set_impl(name, a, size, transpose); }

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
    template<typename T>
    std::enable_if_t<std::is_fundamental_v<T>>
    set_impl(const std::string&, T);

    template<typename T>
    std::enable_if_t<!std::is_fundamental_v<T>>
    set_impl(const std::string&, const T&);

    template<typename T>
    void set_impl(const std::string&, const T[], std::size_t);

    template<typename T>
    void set_impl(const std::string&, const T[], std::size_t, bool);

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

    template<typename T>
    std::enable_if_t<std::is_fundamental_v<T>>
    set_impl(const std::string& name, T v)
    {
        Cache(name);
        auto range = cache_.equal_range(name);
        std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x){ x.second->Set(name, v); });
    }

    template<typename T>
    std::enable_if_t<!std::is_fundamental_v<T>>
    set_impl(const std::string& name, const T& v)
    {
        Cache(name);
        auto range = cache_.equal_range(name);
        std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x){ x.second->Set(name, v); });
    }

    template<typename T>
    void set_impl(const std::string& name, const T a[], std::size_t size)
    {
        Cache(name);
        auto range = cache_.equal_range(name);
        std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x){ x.second->Set(name, a, size); });
    }

    template<typename T>
    void set_impl(const std::string& name, const T a[], std::size_t size, bool transpose)
    {
        Cache(name);
        auto range = cache_.equal_range(name);
        std::for_each(range.first, range.second, [&](decltype(cache_)::value_type& x){ x.second->Set(name, a, size, transpose); });
    }

private:
    UniformPtrSet ups_;
    std::unordered_multimap<std::string, Uniform*> cache_;
};

}   // namespace common::render::shader