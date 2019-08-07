#pragma once
#include <unordered_set>
#include <string>
#include <filesystem>
#include <memory>
#include <GL/glew.h>
#include "../../resource.h"
#include "uniform.h"

namespace common::render::shader
{

class Program final : public Resource<Program>
{
    friend Resource<Program>;
public:
    Program(const std::string& source, GLenum type);
    Program(const std::filesystem::path& filepath, GLenum type);
    Program(const std::filesystem::path& filepath);
    ~Program();

    Program(const Program&) = delete;
    Program& operator = (const Program&) = delete;
    Program(Program&&) = delete;
    Program& operator = (Program&&) = delete;

    GLuint GetProgram() const noexcept { return program_; }
    GLenum GetType() const noexcept { return type_; }

    const Uniform& GetUniform() const noexcept { return *uniform_; }
    Uniform& GetUniform() noexcept { return *uniform_; }

private:
    static const Resource<Program>::string_set_t& allowed_extensions_impl();

private:
    GLuint program_;
    GLenum type_;
    std::unique_ptr<Uniform> uniform_;
};


class ProgramPipeline final
{
public:
    struct ProgramPtrHashByType
    {
        std::size_t operator()(const Program* p) const
        {
            return std::hash<int>()(p->GetType());
        }
    };

    struct ProgramEqualByType
    {
        bool operator()(const Program* p1, const Program* p2) const
        {
            return p1->GetType() == p2->GetType();
        }
    };

    using ProgramPtrSet = std::unordered_set<Program*, ProgramPtrHashByType, ProgramEqualByType>;

public:
    explicit ProgramPipeline(const ProgramPtrSet& pps);
    ~ProgramPipeline();

    ProgramPipeline(const ProgramPipeline&) = delete;
    ProgramPipeline& operator = (const ProgramPipeline&) = delete;
    ProgramPipeline(ProgramPipeline&&) = delete;
    ProgramPipeline& operator = (ProgramPipeline&&) = delete;

    GLuint GetPipeline() const noexcept { return pipeline_; }

    const PipelineUniform& GetPipelineUniform() const noexcept { return *pipeline_uniform_; }
    PipelineUniform& GetPipelineUniform() noexcept { return *pipeline_uniform_; }

    void Bind();
    void Unbind();

private:
    GLuint pipeline_;
    ProgramPtrSet pps_;
    std::unique_ptr<PipelineUniform> pipeline_uniform_;
};

}   // common::render::shader