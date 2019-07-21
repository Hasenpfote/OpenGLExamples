#pragma once
#include <GLFW/glfw3.h>
#include <hasenpfote/math/cmatrix4.h>
#include "../../Common/shader.h"

class Quad final
{
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