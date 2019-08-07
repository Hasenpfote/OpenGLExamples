#include <cassert>
#include <GL/glew.h>
#include "quad.h"

Quad::Quad()
{
}

Quad::~Quad()
{
    if(glIsBuffer(position_buffer_object))
        glDeleteBuffers(1, &position_buffer_object);
    if(glIsBuffer(texcoord_buffer_object))
        glDeleteBuffers(1, &texcoord_buffer_object);
    if(glIsBuffer(index_buffer_object))
        glDeleteBuffers(1, &index_buffer_object);
    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);
    if(glIsSampler(sampler))
        glDeleteSamplers(1, &sampler);
}

void Quad::Initialize()
{
    static float vertices[] =
    {
        -10.0f, -10.0f, 0.0f, // left bottom
         10.0f, -10.0f, 0.0f, // right bottom
         10.0f,  10.0f, 0.0f, // right top
        -10.0f,  10.0f, 0.0f  // left top
    };

    static float coords[] =
    {
#if 0
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
#else
        0.0f, 20.0f,
        20.0f, 20.0f,
        20.0f, 0.0f,
        0.0f, 0.0f
#endif
    };

    static int indices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &position_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &texcoord_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, coords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &index_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6, indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

    auto& rm = System::GetMutableInstance().GetResourceManager();

    texture = rm.GetResource<Texture>("assets/textures/chess_board.png")->GetTexture();

    pipeline = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/quad.vs"),
            rm.GetResource<Program>("assets/shaders/quad.fs")})
            );
}

void Quad::Draw()
{
    using namespace hasenpfote::math;

    const auto& camera = System::GetConstInstance().GetCamera();
    CMatrix4 mvp = camera.GetProjectionMatrix() * camera.GetViewMatrix();

    auto& uniform = pipeline->GetPipelineUniform();
    uniform.SetMatrix4fv("mvp", 1, GL_FALSE, static_cast<GLfloat*>(mvp));
    uniform.Set1i("texture", 0);

    pipeline->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindSampler(0, sampler);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
    pipeline->Unbind();
}