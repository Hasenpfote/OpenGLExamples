#include <cassert>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include "billboard_beam.h"

float BillboardBeam::vertices[] =
{
     0.0f,  1.0f, 0.0f, 0.0f,
    -1.0f,  0.0f, 0.0f, 0.0f,
     1.0f,  0.0f, 0.0f, 0.0f,
    -1.0f,  0.0f, 0.0f, 1.0f,
     1.0f,  0.0f, 0.0f, 1.0f,
     0.0f, -1.0f, 0.0f, 1.0f
};

float BillboardBeam::tcoords[] =
{
    0.0f, 1.0f - 0.0f,
    0.0f, 1.0f - 0.5f,
    0.5f, 1.0f - 0.0f,
    0.0f, 1.0f - 1.0f,
    0.5f, 1.0f - 0.5f,
    0.5f, 1.0f - 1.0f
};

BillboardBeam::BillboardBeam()
{
}

BillboardBeam::~BillboardBeam()
{
    if(glIsBuffer(position_buffer_object))
        glDeleteBuffers(1, &position_buffer_object);
    if(glIsBuffer(texcoord_buffer_object))
        glDeleteBuffers(1, &texcoord_buffer_object);
    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);
    if(glIsSampler(sampler))
        glDeleteSamplers(1, &sampler);
}

void BillboardBeam::Initialize()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &position_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &texcoord_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, tcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    auto& rm = System::GetMutableInstance().GetResourceManager();

    texture = rm.GetResource<Texture>("assets/textures/beam.png")->GetTexture();
    texture = rm.GetResource<Texture>("assets/textures/beam_a.png")->GetTexture();

    pipeline = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/billboard_beam.vs"),
            rm.GetResource<Program>("assets/shaders/billboard_beam.fs")})
            );
}

void BillboardBeam::UpdateMatrices(const glm::mat4& model)
{
    auto& camera = System::GetConstInstance().GetCamera();
    mv = camera.view() * model;
    mvp = camera.proj() * mv;
}

void BillboardBeam::Draw(const glm::vec3& ep1, const glm::vec3& ep2, float size)
{
    static const glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    Draw(ep1, ep2, color, color, size);
}

void BillboardBeam::Draw(const glm::vec3& ep1, const glm::vec3& ep2, const glm::vec4& color, float size)
{
    Draw(ep1, ep2, color, color, size);
}

void BillboardBeam::Draw(const glm::vec3& ep1, const glm::vec3& ep2, const glm::vec4& color1, const glm::vec4& color2, float size)
{
    glm::mat4 ma, mb;
    ComputeBillboardBeamMatrix2(ep1, ep2, mv, ma, mb);

    auto& uniform = pipeline->GetPipelineUniform();
    uniform.Set1f("size", size);
    uniform.SetMatrix4fv("bb[0]", 1, GL_FALSE, glm::value_ptr(ma));
    uniform.SetMatrix4fv("bb[1]", 1, GL_FALSE, glm::value_ptr(mb));
    uniform.Set4fv("color[0]", 1, glm::value_ptr(color1));
    uniform.Set4fv("color[1]", 1, glm::value_ptr(color2));
    uniform.SetMatrix4fv("mvp", 1, GL_FALSE, glm::value_ptr(mvp));
    uniform.Set1i("texture", 0);

    pipeline->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindSampler(0, sampler);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
    pipeline->Unbind();
}

void BillboardBeam::ComputeBillboardBeamMatrix(const glm::vec3& ep1, const glm::vec3& ep2, const glm::mat4& view, glm::mat4& bb1, glm::mat4& bb2)
{
    auto inv = glm::inverse(view);

    auto E = glm::vec3(inv[3]);
    auto F = glm::vec3(inv[2]);

    auto B = ep1 - ep2;
    auto P = glm::cross(B, E);
    auto U = glm::normalize(glm::cross(F, P));

    //auto R = glm::cross(F, U);  // cw
    auto R = glm::cross(U, F);    // ccw

    bb1[0] = glm::vec4(R, 0.0f);
    bb1[1] = glm::vec4(U, 0.0f);
    bb1[2] = glm::vec4(F, 0.0f);
    bb1[3] = glm::vec4(ep1, 1.0f);

    bb2[0] = glm::vec4(R, 0.0f);
    bb2[1] = glm::vec4(U, 0.0f);
    bb2[2] = glm::vec4(F, 0.0f);
    bb2[3] = glm::vec4(ep2, 1.0f);
}

void BillboardBeam::ComputeBillboardBeamMatrix2(const glm::vec3& ep1, const glm::vec3& ep2, const glm::mat4& view, glm::mat4& bb1, glm::mat4& bb2)
{
    auto inv = glm::inverse(view);

    auto B = ep1 - ep2;
    auto d1 = glm::length(B);
    if(d1 > 0.0f)
    {
        auto E = glm::vec3(inv[3]) - ep1;
        auto d2 = glm::length(E);
        if(!(d2 > 0.0f))
        {
            E = glm::vec3(inv[3]) - ep2;
            d2 = E.length();
        }

        glm::vec3 P;
        if(glm::abs(glm::dot(B, E)) < (d1 * d2))
        {
            P = glm::normalize(glm::cross(B, E));
        }
        else
        {
            P = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        auto F = glm::vec3(inv[2]);
        auto U = glm::cross(F, P);
        auto R = glm::cross(U, F);    // ccw

        bb1[0] = glm::vec4(R, 0.0f);
        bb1[1] = glm::vec4(U, 0.0f);
        bb1[2] = glm::vec4(F, 0.0f);
        bb1[3] = glm::vec4(ep1, 1.0f);

        bb2[0] = glm::vec4(R, 0.0f);
        bb2[1] = glm::vec4(U, 0.0f);
        bb2[2] = glm::vec4(F, 0.0f);
        bb2[3] = glm::vec4(ep2, 1.0f);
    }
    else
    {
        // 点となるので通常のビルボード処理へ移行
        bb1[0] = inv[0];
        bb1[1] = inv[1];
        bb1[2] = inv[2];
        bb1[3] = glm::vec4(ep1, 1.0f);

        bb2[0] = inv[0];
        bb2[1] = inv[1];
        bb2[2] = inv[2];
        bb2[3] = glm::vec4(ep2, 1.0f);
    }
}