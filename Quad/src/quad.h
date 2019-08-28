#pragma once
#include <GLFW/glfw3.h>
#include "../../common/system.h"
#include "../../common/render/texture.h"
#include "../../common/render/shader/shader.h"

class Quad final
{
    using System = common::System;
    using Texture = common::render::Texture;
    using Program = common::render::shader::Program;
    using ProgramPipeline = common::render::shader::ProgramPipeline;

public:
    Quad();
    ~Quad();

    void Initialize();
    void Draw();

private:
    GLuint vao;
    GLuint position_buffer_object;
    GLuint texcoord_buffer_object;
    GLuint index_buffer_object;
    GLuint sampler;
    GLuint texture;

    std::unique_ptr<ProgramPipeline> pipeline;
};