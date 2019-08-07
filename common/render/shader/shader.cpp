#include <stdexcept>
#include <fstream>
#include <functional>
#include <hasenpfote/assert.h>
#include "../../logger.h"
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

namespace common::render::shader
{

Program::Program(const std::string& source, GLenum type)
    : program_(0), type_(0)
{
    program_ = create_shader_program(source, type);
    type_ = type;
    uniform_ = std::make_unique<Uniform>(program_);
}

Program::Program(const std::filesystem::path& filepath, GLenum type)
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
    uniform_ = std::make_unique<Uniform>(program_);
}

Program::Program(const std::filesystem::path& filepath)
    : Program(filepath, file_extension_to_shader_type(filepath.extension()))
{
}

Program::~Program()
{
    if(glIsProgram(program_))
        glDeleteProgram(program_);
}

const Resource<Program>::string_set_t& Program::allowed_extensions_impl()
{
    static string_set_t ss({ ".vs", ".tcs", ".tes", ".gs", ".fs" });
    return ss;
}


ProgramPipeline::ProgramPipeline(const ProgramPtrSet& pps)
    : pipeline_(0), pps_(pps)
{
    HASENPFOTE_ASSERT(!pps.empty());

    glGenProgramPipelines(1, &pipeline_);

    PipelineUniform::UniformPtrSet ups;

    for(auto pp : pps_)
    {
        auto type = pp->GetType();
        GLbitfield stage = shader_type_to_stage(type);
        HASENPFOTE_ASSERT_MSG(stage > 0, "Unknown type");

        auto program = pp->GetProgram();
        glUseProgramStages(pipeline_, stage, program);
        HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline_));

        ups.emplace(&pp->GetUniform());
    }

    pipeline_uniform_ = std::make_unique<PipelineUniform>(ups);
}

ProgramPipeline::~ProgramPipeline()
{
    if(glIsProgramPipeline(pipeline_))
        glDeleteProgramPipelines(1, &pipeline_);
}

void ProgramPipeline::Bind()
{
    HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline_));
    glUseProgram(0);
    glBindProgramPipeline(pipeline_);
}

void ProgramPipeline::Unbind()
{
    glBindProgramPipeline(0);
}

}   // namespace common::render::shader