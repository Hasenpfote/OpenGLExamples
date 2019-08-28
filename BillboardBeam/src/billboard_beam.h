#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "../../common/system.h"
#include "../../common/render/texture.h"
#include "../../common/render/shader/shader.h"

class BillboardBeam final
{
    using System = common::System;
    using Texture = common::render::Texture;
    using Program = common::render::shader::Program;
    using ProgramPipeline = common::render::shader::ProgramPipeline;

public:
    BillboardBeam();
    ~BillboardBeam();

    void Initialize();
    void UpdateMatrices(const glm::mat4& model);
    void Draw(const glm::vec3& ep1, const glm::vec3& ep2, float size = 1.0f);
    void Draw(const glm::vec3& ep1, const glm::vec3& ep2, const glm::vec4& color, float size = 1.0f);
    void Draw(const glm::vec3& ep1, const glm::vec3& ep2, const glm::vec4& color1, const glm::vec4& color2, float size = 1.0f);

private:
    static void ComputeBillboardBeamMatrix(const glm::vec3& ep1, const glm::vec3& ep2, const glm::mat4& view, glm::mat4& bb1, glm::mat4& bb2);
    static void ComputeBillboardBeamMatrix2(const glm::vec3& ep1, const glm::vec3& ep2, const glm::mat4& view, glm::mat4& bb1, glm::mat4& bb2);

private:
    static float vertices[];
    static float tcoords[];

    GLuint vao;
    GLuint position_buffer_object;
    GLuint texcoord_buffer_object;
    GLuint sampler;
    GLuint texture;

    glm::mat4 mv;
    glm::mat4 mvp;

    std::unique_ptr<ProgramPipeline> pipeline;
};