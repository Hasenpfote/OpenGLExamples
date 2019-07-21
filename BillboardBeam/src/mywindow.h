#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <hasenpfote/math/quaternion.h>
#include <hasenpfote/math/vector3.h>
#include "../../Common/window.h"
#include "../../Common/sdf_text.h"
#include "billboard_beam.h"

class MyWindow final : public Window
{
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

    std::vector<hasenpfote::math::Vector3> ctrl_points;
    std::deque<hasenpfote::math::Vector3> points;
    float t;
};