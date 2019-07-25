#include <cassert>
#include <GL/glew.h>
#include "../../common/system.h"
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

    //texture = System::GetConstInstance().GetTextureManager().GetTexture("assets/textures/beam.png");
    texture = System::GetConstInstance().GetTextureManager().GetTexture("assets/textures/beam_a.png");

    auto& man = System::GetConstInstance().GetShaderManager();
    pipeline.Create();
    pipeline.SetShaderProgram(man.GetShaderProgram("assets/shaders/billboard_beam.vs"));
    pipeline.SetShaderProgram(man.GetShaderProgram("assets/shaders/billboard_beam.fs"));
}

void BillboardBeam::UpdateMatrices(const hasenpfote::math::CMatrix4& model)
{
    auto& camera = System::GetConstInstance().GetCamera();
    mv = camera.GetViewMatrix() * model;
    mvp = camera.GetProjectionMatrix() * mv;
}

void BillboardBeam::Draw(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, float size)
{
    using namespace hasenpfote::math;
    static const Vector4 color(1.0f, 1.0f, 1.0f, 1.0f);
    Draw(ep1, ep2, color, color, size);
}

void BillboardBeam::Draw(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, const hasenpfote::math::Vector4& color, float size)
{
    Draw(ep1, ep2, color, color, size);
}

void BillboardBeam::Draw(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, const hasenpfote::math::Vector4& color1, const hasenpfote::math::Vector4& color2, float size)
{
    using namespace hasenpfote::math;

    CMatrix4 ma, mb;
    ComputeBillboardBeamMatrix2(ep1, ep2, mv, ma, mb);

    pipeline.SetUniform1f("size", size);
    pipeline.SetUniformMatrix4fv("bb[0]", 1, GL_FALSE, static_cast<GLfloat*>(ma));
    pipeline.SetUniformMatrix4fv("bb[1]", 1, GL_FALSE, static_cast<GLfloat*>(mb));
    pipeline.SetUniform4fv("color[0]", 1, static_cast<const GLfloat*>(color1));
    pipeline.SetUniform4fv("color[1]", 1, static_cast<const GLfloat*>(color2));
    pipeline.SetUniformMatrix4fv("mvp", 1, GL_FALSE, static_cast<GLfloat*>(mvp));
    pipeline.SetUniform1i("texture", 0);

    pipeline.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);

    pipeline.Unbind();
}

void BillboardBeam::ComputeBillboardBeamMatrix(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, const hasenpfote::math::CMatrix4& view, hasenpfote::math::CMatrix4& bb1, hasenpfote::math::CMatrix4& bb2)
{
    using namespace hasenpfote::math;
#if 1
    CMatrix4 inv = CMatrix4::Inverse(view);
 #if 0
    Vector3 E = Vector3(inv.m14, inv.m24, inv.m34) - ep1;
    Vector3 F = Vector3(inv.m13, inv.m23, inv.m33);
 #else
    Vector3 E = Vector3(inv.GetColumn(3));
    Vector3 F = Vector3(inv.GetColumn(2));
#endif
#else
    Vector3 pos;
    pos.x = -(view.m11 * view.m14 + view.m21 * view.m24 + view.m31 * view.m34);
    pos.y = -(view.m12 * view.m14 + view.m22 * view.m24 + view.m32 * view.m34);
    pos.z = -(view.m13 * view.m14 + view.m23 * view.m24 + view.m33 * view.m34);
    Vector3 E = pos - ep1;
    Vector3 F = Vector3(view.m31, view.m32, view.m33);
#endif
    Vector3 B = ep1 - ep2;
    Vector3 P = Vector3::CrossProduct(B, E);
    Vector3 U = Vector3::CrossProduct(F, P);
    U.Normalize();
    //Vector3 R = Vector3::CrossProduct(F, U);  // cw
    Vector3 R = Vector3::CrossProduct(U, F);    // ccw
#if 0
    bb1 = CMatrix4(
        R.GetX(), U.GetX(), F.GetX(), ep1.GetX(),
        R.GetY(), U.GetY(), F.GetY(), ep1.GetY(),
        R.GetZ(), U.GetZ(), F.GetZ(), ep1.GetZ(),
        0.0f, 0.0f, 0.0f, 1.0f
    );
    bb2 = CMatrix4(
        R.GetX(), U.GetX(), F.GetX(), ep2.GetX(),
        R.GetY(), U.GetY(), F.GetY(), ep2.GetY(),
        R.GetZ(), U.GetZ(), F.GetZ(), ep2.GetZ(),
        0.0f, 0.0f, 0.0f, 1.0f
    );
#else
    bb1 = CMatrix4(Vector4(R, 0.0f), Vector4(U, 0.0f), Vector4(F, 0.0f), Vector4(ep1, 1.0f));
    bb2 = CMatrix4(Vector4(R, 0.0f), Vector4(U, 0.0f), Vector4(F, 0.0f), Vector4(ep2, 1.0f));
#endif
}

void BillboardBeam::ComputeBillboardBeamMatrix2(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, const hasenpfote::math::CMatrix4& view, hasenpfote::math::CMatrix4& bb1, hasenpfote::math::CMatrix4& bb2)
{
    using namespace hasenpfote::math;

    CMatrix4 inv = CMatrix4::Inverse(view);

    Vector3 B = ep1 - ep2;
    float d1 = B.Magnitude();
    if(d1 > 0.0f){
#if 0
        Vector3 E = Vector3(inv.m14, inv.m24, inv.m34) - ep1;
#else
        Vector3 E = Vector3(inv.GetColumn(3)) - ep1;
#endif
        float d2 = E.Magnitude();
        if(!(d2 > 0.0f)){
#if 0
            E = Vector3(inv.m14, inv.m24, inv.m34) - ep2;
#else
            E = Vector3(inv.GetColumn(3)) - ep2;
#endif
            d2 = E.Magnitude();
        }

        Vector3 P;
        if(std::fabsf(Vector3::DotProduct(B, E)) < (d1 * d2)){
            P = Vector3::CrossProduct(B, E);
            P.Normalize();
        }
        else{
            P = Vector3(0.0f, 1.0f, 0.0f);
        }
#if 0
        Vector3 F = Vector3(inv.m13, inv.m23, inv.m33);
#else
        Vector3 F = Vector3(inv.GetColumn(2));
#endif
        Vector3 U = Vector3::CrossProduct(F, P);
        Vector3 R = Vector3::CrossProduct(U, F);    // ccw
#if 0
        bb1 = CMatrix4(
            R.GetX(), U.GetX(), F.GetX(), ep1.GetX(),
            R.GetY(), U.GetY(), F.GetY(), ep1.GetY(),
            R.GetZ(), U.GetZ(), F.GetZ(), ep1.GetZ(),
            0.0f, 0.0f, 0.0f, 1.0f
        );
        bb2 = CMatrix4(
            R.GetX(), U.GetX(), F.GetX(), ep2.GetX(),
            R.GetY(), U.GetY(), F.GetY(), ep2.GetY(),
            R.GetZ(), U.GetZ(), F.GetZ(), ep2.GetZ(),
            0.0f, 0.0f, 0.0f, 1.0f
        );
#else
        bb1 = CMatrix4(Vector4(R, 0.0f), Vector4(U, 0.0f), Vector4(F, 0.0f), Vector4(ep1, 1.0f));
        bb2 = CMatrix4(Vector4(R, 0.0f), Vector4(U, 0.0f), Vector4(F, 0.0f), Vector4(ep2, 1.0f));
#endif
    }
    else{
        // 点となるので通常のビルボード処理へ移行
#if 0
        bb1 = CMatrix4(
            inv.m11, inv.m12, inv.m13, ep1.GetX(),
            inv.m21, inv.m22, inv.m23, ep1.GetY(),
            inv.m31, inv.m32, inv.m33, ep1.GetZ(),
            0.0f, 0.0f, 0.0f, 1.0f
        );
        bb2 = CMatrix4(
            inv.m11, inv.m12, inv.m13, ep2.GetX(),
            inv.m21, inv.m22, inv.m23, ep2.GetY(),
            inv.m31, inv.m32, inv.m33, ep2.GetZ(),
            0.0f, 0.0f, 0.0f, 1.0f
        );
#else
        bb1 = CMatrix4(inv.GetColumn(0), inv.GetColumn(1), inv.GetColumn(2), Vector4(ep1, 1.0f));
        bb2 = CMatrix4(inv.GetColumn(0), inv.GetColumn(1), inv.GetColumn(2), Vector4(ep2, 1.0f));
#endif
    }
}