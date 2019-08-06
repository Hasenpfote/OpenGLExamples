#pragma once
#include <GLFW/glfw3.h>
#include <hasenpfote/math/cmatrix4.h>
#include <hasenpfote/math/vector3.h>
#include "../../common/shader.h"

class Terrain final
{
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

    hasenpfote::math::Vector3 light_direction;

    common::ShaderPipeline pipeline1;
    common::ShaderPipeline pipeline2;
};