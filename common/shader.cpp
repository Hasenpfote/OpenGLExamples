#include <fstream>
#include <functional>
#include <hasenpfote/assert.h>
#include "logger.h"
#include "shader.h"

static std::unordered_map<std::string, GLenum> make_extension_map();

const std::unordered_map<std::string, GLenum> ShaderManager::ext_type = make_extension_map();

ShaderProgram::ShaderProgram()
{
    program = 0;
    type = 0;
}

ShaderProgram::~ShaderProgram()
{
    if(program > 0)
        glDeleteProgram(program);
}

GLint ShaderProgram::GetUniformLocation(const std::string& name) const
{
    HASENPFOTE_ASSERT(program > 0);

    decltype(uniform_location_cache)::const_iterator it = uniform_location_cache.find(name);
    if(it != uniform_location_cache.cend()){
        return it->second;
    }
    auto loc = glGetUniformLocation(program, name.c_str());
    if(loc >= 0){
        uniform_location_cache.insert(std::make_pair(name, loc));  // caching
    }
    return loc;
}

GLuint ShaderProgram::GetUniformBlockIndex(const std::string& name) const
{
    HASENPFOTE_ASSERT(program > 0);

    decltype(uniform_block_index_cache)::const_iterator it = uniform_block_index_cache.find(name);
    if(it != uniform_block_index_cache.cend()){
        return it->second;
    }
    auto index = glGetUniformBlockIndex(program, name.c_str());
    if(index != GL_INVALID_INDEX){
        uniform_block_index_cache.insert(std::make_pair(name, index));  // caching
    }
    return index;
}

bool ShaderProgram::Create(GLenum type, const std::string& source)
{
    Release();

    auto s = source.c_str();
    GLuint program = glCreateShaderProgramv(type, 1, &s);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE){
        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        if(log_length > 0){
            std::vector<GLchar> log(log_length);
            GLsizei length = 0;
            glGetProgramInfoLog(program, log_length, &length, log.data());
            std::istringstream iss(std::string(log.data()));
            std::string field;
            while(std::getline(iss, field, '\n')){
                LOG_E("ProgramInfoLog:" << field);
            }
        }
        glDeleteProgram(program);
        return false;
    }
    this->program = program;
    this->type = type;
    return true;
}

bool ShaderProgram::Create(GLenum type, const std::filesystem::path& filepath)
{
    Release();

    std::ifstream ifs(filepath.string(), std::ios::in | std::ios::binary);
    if(ifs.fail()){
        LOG_E("Could not read shader: " << filepath.filename().string());
        return false;
    }
    std::istreambuf_iterator<GLchar> it(ifs);
    std::istreambuf_iterator<GLchar> last;
    std::string source(it, last);
    return Create(type, source);
}

void ShaderProgram::Release()
{
    if(program > 0){
        glDeleteProgram(program);
    }
    program = 0;
    type = 0;
    uniform_location_cache.clear();
    uniform_block_index_cache.clear();
}

void ShaderProgram::SetUniform1f(const std::string& name, GLfloat v0)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1f(program, loc, v0);
}

void ShaderProgram::SetUniform2f(const std::string& name, GLfloat v0, GLfloat v1)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2f(program, loc, v0, v1);
}

void ShaderProgram::SetUniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3f(program, loc, v0, v1, v2);
}

void ShaderProgram::SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4f(program, loc, v0, v1, v2, v3);
}

void ShaderProgram::SetUniform1i(const std::string& name, GLint v0)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1i(program, loc, v0);
}

void ShaderProgram::SetUniform2i(const std::string& name, GLint v0, GLint v1)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2i(program, loc, v0, v1);
}

void ShaderProgram::SetUniform3i(const std::string& name, GLint v0, GLint v1, GLint v2)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3i(program, loc, v0, v1, v2);
}

void ShaderProgram::SetUniform4i(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4i(program, loc, v0, v1, v2, v3);
}

void ShaderProgram::SetUniform1ui(const std::string& name, GLuint v0)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1ui(program, loc, v0);
}

void ShaderProgram::SetUniform2ui(const std::string& name, GLuint v0, GLuint v1)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2ui(program, loc, v0, v1);
}

void ShaderProgram::SetUniform3ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3ui(program, loc, v0, v1, v2);
}

void ShaderProgram::SetUniform4ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4ui(program, loc, v0, v1, v2, v3);
}

void ShaderProgram::SetUniform1fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1fv(program, loc, count, value);
}

void ShaderProgram::SetUniform2fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2fv(program, loc, count, value);
}

void ShaderProgram::SetUniform3fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3fv(program, loc, count, value);
}

void ShaderProgram::SetUniform4fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4fv(program, loc, count, value);
}

void ShaderProgram::SetUniform1iv(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1iv(program, loc, count, value);
}

void ShaderProgram::SetUniform2iv(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2iv(program, loc, count, value);
}

void ShaderProgram::SetUniform3iv(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3iv(program, loc, count, value);
}

void ShaderProgram::SetUniform4iv(const std::string& name, GLsizei count, const GLint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4iv(program, loc, count, value);
}

void ShaderProgram::SetUniform1uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform1uiv(program, loc, count, value);
}

void ShaderProgram::SetUniform2uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform2uiv(program, loc, count, value);
}

void ShaderProgram::SetUniform3uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform3uiv(program, loc, count, value);
}

void ShaderProgram::SetUniform4uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniform4uiv(program, loc, count, value);
}

void ShaderProgram::SetUniformMatrix2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2fv(program, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3fv(program, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4fv(program, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix2x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2x3fv(program, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix3x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3x2fv(program, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix2x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix2x4fv(program, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix4x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4x2fv(program, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix3x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix3x4fv(program, loc, count, transpose, value);
}

void ShaderProgram::SetUniformMatrix4x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    const auto loc = GetUniformLocation(name);
    HASENPFOTE_ASSERT_MSG(loc >= 0, "Could not find uniform variable `" << name << "` in shader.");
    glProgramUniformMatrix4x3fv(program, loc, count, transpose, value);
}

static std::unordered_map<std::string, GLenum> make_extension_map()
{
    std::unordered_map<std::string, GLenum> result;
    result.insert(std::make_pair(".vs", GL_VERTEX_SHADER));
    result.insert(std::make_pair(".tcs", GL_TESS_CONTROL_SHADER));
    result.insert(std::make_pair(".tes", GL_TESS_EVALUATION_SHADER));
    result.insert(std::make_pair(".gs", GL_GEOMETRY_SHADER));
    result.insert(std::make_pair(".fs", GL_FRAGMENT_SHADER));
    return result;
}

ShaderPipeline::ShaderPipeline()
{
    pipeline = 0;
}

ShaderPipeline::~ShaderPipeline()
{
    if(glIsProgramPipeline(pipeline))
        glDeleteProgramPipelines(1, &pipeline);
}

void ShaderPipeline::Create()
{
    if(!glIsProgramPipeline(pipeline))
        glGenProgramPipelines(1, &pipeline);
}

void ShaderPipeline::Release()
{
    if(glIsProgramPipeline(pipeline)){
        glDeleteProgramPipelines(1, &pipeline);
    }
    pipeline = 0;
    shader_program.clear();
    uniform_cache.clear();
}

void ShaderPipeline::Bind()
{
    HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline));
    glUseProgram(0);
    glBindProgramPipeline(pipeline);
}

void ShaderPipeline::Unbind()
{
    glBindProgramPipeline(0);
}

void ShaderPipeline::SetShaderProgram(const ShaderProgram* sp)
{
    HASENPFOTE_ASSERT(sp != nullptr);
    //HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline));
    GLbitfield stage;
    switch(sp->GetType()){
    case GL_VERTEX_SHADER:
        stage = GL_VERTEX_SHADER_BIT;
        break;
    case GL_TESS_CONTROL_SHADER:
        stage = GL_TESS_CONTROL_SHADER_BIT;
        break;
    case GL_TESS_EVALUATION_SHADER:
        stage = GL_TESS_EVALUATION_SHADER_BIT;
        break;
    case GL_GEOMETRY_SHADER:
        stage = GL_GEOMETRY_SHADER_BIT;
        break;
    case GL_FRAGMENT_SHADER:
        stage = GL_FRAGMENT_SHADER_BIT;
        break;
    default:
        HASENPFOTE_ASSERT_MSG(false, "Unknown type");
        break;
    }
    glUseProgramStages(pipeline, stage, sp->GetProgram());
    HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline));

    decltype(shader_program)::const_iterator it = shader_program.find(sp->GetType());
    if(it != shader_program.cend()){
        shader_program.erase(it);
        uniform_cache.clear();
    }
    shader_program.insert(std::make_pair(sp->GetType(), const_cast<ShaderProgram*>(sp)));
}

ShaderProgram* ShaderPipeline::GetShaderProgram(GLenum type)
{
    decltype(shader_program)::const_iterator it = shader_program.find(type);
    return (it != shader_program.cend())? it->second : nullptr;
}

const ShaderProgram* ShaderPipeline::GetShaderProgram(GLenum type) const
{
    decltype(shader_program)::const_iterator it = shader_program.find(type);
    return (it != shader_program.cend())? it->second : nullptr;
}

void ShaderPipeline::ResetShaderPrograms()
{
    HASENPFOTE_ASSERT(glIsProgramPipeline(pipeline));
    glUseProgramStages(pipeline, GL_ALL_SHADER_BITS, 0);
    shader_program.clear();
    uniform_cache.clear();
}

void ShaderPipeline::SetUniform1f(const std::string& name, GLfloat v0)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform1f(name, v0);
    }
}

void ShaderPipeline::SetUniform2f(const std::string& name, GLfloat v0, GLfloat v1)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform2f(name, v0, v1);
    }
}

void ShaderPipeline::SetUniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform3f(name, v0, v1, v2);
    }
}

void ShaderPipeline::SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform4f(name, v0, v1, v2, v3);
    }
}

void ShaderPipeline::SetUniform1i(const std::string& name, GLint v0)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform1i(name, v0);
    }
}

void ShaderPipeline::SetUniform2i(const std::string& name, GLint v0, GLint v1)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform2i(name, v0, v1);
    }
}

void ShaderPipeline::SetUniform3i(const std::string& name, GLint v0, GLint v1, GLint v2)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform3i(name, v0, v1, v2);
    }
}

void ShaderPipeline::SetUniform4i(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform4i(name, v0, v1, v2, v3);
    }
}

void ShaderPipeline::SetUniform1ui(const std::string& name, GLuint v0)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform1ui(name, v0);
    }
}

void ShaderPipeline::SetUniform2ui(const std::string& name, GLuint v0, GLuint v1)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform2ui(name, v0, v1);
    }
}

void ShaderPipeline::SetUniform3ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform3ui(name, v0, v1, v2);
    }
}

void ShaderPipeline::SetUniform4ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform4ui(name, v0, v1, v2, v3);
    }
}

void ShaderPipeline::SetUniform1fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform1fv(name, count, value);
    }
}

void ShaderPipeline::SetUniform2fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform2fv(name, count, value);
    }
}

void ShaderPipeline::SetUniform3fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform3fv(name, count, value);
    }
}

void ShaderPipeline::SetUniform4fv(const std::string& name, GLsizei count, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform4fv(name, count, value);
    }
}

void ShaderPipeline::SetUniform1iv(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform1iv(name, count, value);
    }
}

void ShaderPipeline::SetUniform2iv(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform2iv(name, count, value);
    }
}

void ShaderPipeline::SetUniform3iv(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform3iv(name, count, value);
    }
}

void ShaderPipeline::SetUniform4iv(const std::string& name, GLsizei count, const GLint* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform4iv(name, count, value);
    }
}

void ShaderPipeline::SetUniform1uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform1uiv(name, count, value);
    }
}

void ShaderPipeline::SetUniform2uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform2uiv(name, count, value);
    }
}

void ShaderPipeline::SetUniform3uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform3uiv(name, count, value);
    }
}

void ShaderPipeline::SetUniform4uiv(const std::string& name, GLsizei count, const GLuint* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniform4uiv(name, count, value);
    }
}

void ShaderPipeline::SetUniformMatrix2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix2fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix3fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix4fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix2x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix2x3fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix3x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix3x2fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix2x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix2x4fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix4x2fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix4x2fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix3x4fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix3x4fv(name, count, transpose, value);
    }
}

void ShaderPipeline::SetUniformMatrix4x3fv(const std::string& name, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    Cache(name);
    auto pair = uniform_cache.equal_range(name);
    for(auto it = pair.first; it != pair.second; it++){
        it->second->SetUniformMatrix4x3fv(name, count, transpose, value);
    }
}

void ShaderPipeline::Cache(const std::string& name)
{
    if(uniform_cache.find(name) != uniform_cache.cend())
        return;

    for(const auto& pair : shader_program){
        if(pair.second->GetUniformLocation(name) >= 0){
            uniform_cache.insert(std::make_pair(name, pair.second));
        }
    }
    HASENPFOTE_ASSERT_MSG(uniform_cache.find(name) != uniform_cache.cend(), "Could not find uniform variable `" << name << "` in shaders.");
}

bool ShaderManager::CreateShaderProgramFromFile(const std::filesystem::path& filepath)
{
    LOG_I("Creating shader program from file `" << filepath.string() << "`.");

    if(GetShaderProgram(filepath))
    {
        LOG_E("Shader program with the name `" << filepath.string() << "` already exists.");
        return true;
    }

    decltype(ext_type)::const_iterator it = ext_type.find(filepath.extension().string());
    if(it == ext_type.cend())
    {
        return false;
    }

    auto program = std::make_unique<ShaderProgram>();
    if(!program->Create(it->second, filepath))
    {
        LOG_E("Failed to create shader program.");
        return false;
    }

    LOG_I("Shader program created successfully. [id=" << program->GetProgram() << "]");

    const auto hash = std::hash<std::string>()(filepath.string());
    shader_program.insert(std::make_pair(hash, std::move(program)));

    return true;
}

bool ShaderManager::LoadShaderProgram(const std::filesystem::path& filepath)
{
    return CreateShaderProgramFromFile(filepath);
}

void ShaderManager::LoadShaderPrograms(const std::filesystem::path& directory)
{
    const auto& extensions = ext_type;

    std::vector<std::filesystem::path> filepaths;
    auto func = [&filepaths, &extensions](const std::filesystem::path& filepath){
        if(std::filesystem::is_regular_file(filepath)){
            decltype(ext_type)::const_iterator it = extensions.find(filepath.extension().string());
            if(it != extensions.cend()){
                filepaths.push_back(filepath.generic_string());
            }
        }
    };

    std::for_each(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator(), func);
    for(const auto& filepath : filepaths){
        LoadShaderProgram(filepath);
    }
}

void ShaderManager::DeleteShaderProgram(std::size_t hash)
{
    auto it = shader_program.find(hash);
    if(it != shader_program.end()){
        it->second.reset();
        shader_program.erase(it);
    }
}

void ShaderManager::DeleteShaderProgram(const std::filesystem::path& filepath)
{
    const auto hash = std::hash<std::string>()(filepath.string());
    DeleteShaderProgram(hash);
}

void ShaderManager::DeleteShaderPrograms()
{
    for(auto& pair : shader_program){
        pair.second.reset();
    }
    shader_program.clear();
}

ShaderProgram* ShaderManager::GetShaderProgram(std::size_t hash)
{
    decltype(shader_program)::const_iterator it = shader_program.find(hash);
    if(it != shader_program.cend()){
        return it->second.get();
    }
    return nullptr;
}

const ShaderProgram* ShaderManager::GetShaderProgram(std::size_t hash) const
{
    decltype(shader_program)::const_iterator it = shader_program.find(hash);
    if(it != shader_program.cend()){
        return it->second.get();
    }
    return nullptr;
}

ShaderProgram* ShaderManager::GetShaderProgram(const std::filesystem::path& filepath)
{
    const auto hash = std::hash<std::string>()(filepath.string());
    return GetShaderProgram(hash);
}

const ShaderProgram* ShaderManager::GetShaderProgram(const std::filesystem::path& filepath) const
{
    const auto hash = std::hash<std::string>()(filepath.string());
    return GetShaderProgram(hash);
}