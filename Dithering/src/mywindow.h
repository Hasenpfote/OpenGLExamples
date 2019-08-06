#pragma once
#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>
#include <array>
#include <tuple>
#include <hasenpfote/math/quaternion.h>
#include "../../common/window.h"
#include "../../common/sdf_text.h"
#include "../../common/framebuffer.h"
#include "fullscreen_pass_geometry.h"

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

    void RecreateResources(int width, int height);
    void DrawTextLines(std::vector<std::string> text_lines);

    void DrawFullScreenQuad(GLuint texture);

    void PassDithering(FrameBuffer* input, FrameBuffer* output = nullptr);
    void PassApply(FrameBuffer* input, FrameBuffer* output = nullptr);

private:
    std::unique_ptr<SDFText> text;
    std::unique_ptr<FullscreenPassGeometry> fs_pass_geom;
    GLuint nearest_sampler;
    GLuint linear_sampler;

    std::vector<std::tuple<GLuint, std::filesystem::path>> selectable_textures;
    int selected_texture_index;

    common::ShaderPipeline pipeline_fullscreen_quad;
    common::ShaderPipeline pipeline_dithering;
    common::ShaderPipeline pipeline_apply;

    std::unique_ptr<FrameBuffer> scene_rt;

    bool is_dithering_enabled;
    int selected_dither_setting_index;
    int dithering_mode;
};