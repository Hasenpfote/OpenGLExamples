#include <cassert>
#include <GL/glew.h>
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "../../common/system.h"
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
    if(glIsTexture(texture))
        glDeleteTextures(1, &texture);
}

void Quad::Initialize()
{
    static float vertices[] =
    {
        //
        -20.0f, -10.0f, 0.0f, // left bottom
          0.0f, -10.0f, 0.0f, // right bottom
          0.0f,  10.0f, 0.0f, // right top
        -20.0f,  10.0f, 0.0f, // left top
        //
         0.0f, -10.0f, 0.0f, // left bottom
        20.0f, -10.0f, 0.0f, // right bottom
        20.0f,  10.0f, 0.0f, // right top
         0.0f,  10.0f, 0.0f  // left top
    };

    static float coords[] =
    {
        // s, t, layer
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        //
        0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };

    static int indices[] =
    {
        //
        0, 1, 2,
        0, 2, 3,
        //
        4, 5, 6,
        4, 6, 7
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &position_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &texcoord_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &index_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if 0
    texture = System::GetConstInstance().GetTextureManager().GetTexture("assets/textures/rate-star-button.png");
#else
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 128, 128, 2);
    //glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 128, 128, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    {
        int width, height, component;
        unsigned char* pixels = stbi_load("assets/textures/rate-star-button.png", &width, &height, &component, STBI_default);
        if(pixels != nullptr)
        {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            stbi_image_free(pixels);
        }
    }
    {
        int width, height, component;
        unsigned char* pixels = stbi_load("assets/textures/plain-heart.png", &width, &height, &component, STBI_default);
        if(pixels != nullptr)
        {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            stbi_image_free(pixels);
        }
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
#endif
    auto& rm = System::GetConstInstance().GetResourceManager();

    pipeline.Create();
    pipeline.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/quad.vs"));
    pipeline.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/quad.fs"));
}

void Quad::Draw()
{
    using namespace hasenpfote::math;

    const auto& camera = System::GetConstInstance().GetCamera();
    CMatrix4 mvp = camera.GetProjectionMatrix() * camera.GetViewMatrix();

    pipeline.SetUniformMatrix4fv("mvp", 1, GL_FALSE, static_cast<GLfloat*>(mvp));
    pipeline.SetUniform1i("texture", 0);

    pipeline.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glBindSampler(0, sampler);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);

    pipeline.Unbind();
}