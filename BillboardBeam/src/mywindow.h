#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <glm/glm.hpp>
#include "../../common/window.h"
#include "../../common/system.h"
#include "../../common/render/texture.h"
#include "../../common/render/shader/shader.h"
#include "../../common/render/text/sdf_text.h"
#include "billboard_beam.h"

class MyWindow final : public common::Window
{
    using System = common::System;
    using Texture = common::render::Texture;
    using Program = common::render::shader::Program;
    using Pipeline = common::render::shader::ProgramPipeline;
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

    void DrawCube();
    void DrawLines();
    void DrawPoints();
    void DrawCurve();

private:
    std::unique_ptr<SDFText> text;
    BillboardBeam bb;
    float theta;

    std::vector<glm::vec3> ctrl_points;
    std::deque<glm::vec3> points;
    float t;
};