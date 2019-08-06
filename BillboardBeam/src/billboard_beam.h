#pragma once
#include <GLFW/glfw3.h>
#include <hasenpfote/math/vector3.h>
#include <hasenpfote/math/vector4.h>
#include <hasenpfote/math/cmatrix4.h>
#include "../../common/system.h"

class BillboardBeam final
{
    using System = common::System;
    using Texture = common::render::Texture;
    using ShaderProgram = common::render::ShaderProgram;
    using ShaderPipeline = common::render::ShaderPipeline;

public:
    BillboardBeam();
    ~BillboardBeam();

    void Initialize();
    void UpdateMatrices(const hasenpfote::math::CMatrix4& model);
    void Draw(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, float size = 1.0f);
    void Draw(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, const hasenpfote::math::Vector4& color, float size = 1.0f);
    void Draw(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, const hasenpfote::math::Vector4& color1, const hasenpfote::math::Vector4& color2, float size = 1.0f);

private:
    static void ComputeBillboardBeamMatrix(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, const hasenpfote::math::CMatrix4& view, hasenpfote::math::CMatrix4& bb1, hasenpfote::math::CMatrix4& bb2);
    static void ComputeBillboardBeamMatrix2(const hasenpfote::math::Vector3& ep1, const hasenpfote::math::Vector3& ep2, const hasenpfote::math::CMatrix4& view, hasenpfote::math::CMatrix4& bb1, hasenpfote::math::CMatrix4& bb2);

private:
    static float vertices[];
    static float tcoords[];

    GLuint vao;
    GLuint position_buffer_object;
    GLuint texcoord_buffer_object;
    GLuint sampler;
    GLuint texture;

    hasenpfote::math::CMatrix4 mv;
    hasenpfote::math::CMatrix4 mvp;

    ShaderPipeline pipeline;
};