﻿#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../../common/window.h"
#include "../../common/system.h"
#include "../../common/render/texture.h"
#include "../../common/render/shader/shader.h"
#include "../../common/render/text/sdf_text.h"
#include "model.h"

class MyWindow final : public common::Window
{
    using System = common::System;
    using Texture = common::render::Texture;
    using Program = common::render::shader::Program;
    using ProgramPipeline = common::render::shader::ProgramPipeline;
    using Font = common::render::text::Font;
    using SDFText = common::render::text::SDFText;
    using SDFTextRenderer = common::render::text::SDFTextRenderer;
    using Camera = common::render::SimpleCamera;

public:
    MyWindow();
    ~MyWindow();

private:
    void Setup() override;
    void Cleanup() override;

    void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) override;
    void OnMouseMove(GLFWwindow* window, double xpos, double ypos)  override;
    void OnMouseButton(GLFWwindow* window, int button, int action, int mods)  override;
    void OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset) override;
    void OnResizeFramebuffer(GLFWwindow* window, int width, int height) override;
    void OnResizeWindow(GLFWwindow* window, int width, int height) override;

    void OnUpdate(double dt) override;
    void OnRender() override;

private:
    std::unique_ptr<SDFText> text;
    Model model;
    bool is_draw_joints_enabled = false;
    //ArcBall arcball;
    glm::quat rot;

    struct CommonMatrices
    {
        glm::mat4 view;
        glm::mat4 projection;
    } common_matrices;

    GLuint ubo;
};