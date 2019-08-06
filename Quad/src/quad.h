#pragma once
#include <GLFW/glfw3.h>
#include <hasenpfote/math/cmatrix4.h>
#include "../../common/system.h"

class Quad final
{
    using System = common::System;
    using Texture = common::render::Texture;
    using ShaderProgram = common::render::ShaderProgram;
    using ShaderPipeline = common::render::ShaderPipeline;

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

    ShaderPipeline pipeline;
};