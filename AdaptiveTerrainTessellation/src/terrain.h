#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "../../common/system.h"
#include "../../common/render/texture.h"
#include "../../common/render/shader/shader.h"

class Terrain final
{
    using System = common::System;
    using Texture = common::render::Texture;
    using Program = common::render::shader::Program;
    using ProgramPipeline = common::render::shader::ProgramPipeline;

public:
    enum class DrawMode
    {
        Solid,
        Wireframe
    };

public:
    Terrain();
    ~Terrain();

    void Initialize();
    void Draw();
    void SetDrawMode(DrawMode mode){ draw_mode = mode; }
    DrawMode GetDrawMode() const { return draw_mode; }

    float GetHorizontalScale(){ return horizontal_scale; }
    float GetVerticalScale(){ return vertical_scale; }

private:
    void DrawSolid();
    void DrawWireFrame();

private:
    GLuint vao;
    GLuint texcoord_buffer_object;
    GLuint index_buffer_object;
    GLuint sampler;
    int num_indices;

    GLuint diffuse_map;
    GLuint height_map;

    float lod_factor;
    float horizontal_scale;
    float vertical_scale;

    DrawMode draw_mode;

    glm::vec3 light_direction;

    std::unique_ptr<ProgramPipeline> pipeline1;
    std::unique_ptr<ProgramPipeline> pipeline2;
};