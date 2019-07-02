﻿#pragma once
#include <iostream>
#include <hasenpfote/math/quaternion.h>
#include "../../Common/window.h"
#include "../../Common/sdf_text.h"
#include "../../Common/framebuffer.h"
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

    void DrawFullScreenQuad();
    void PassHighLuminanceRegionExtraction(GLuint texture, FrameBuffer* fb);
    void PassDownsampling2x2(GLuint texture, FrameBuffer* fb);
    void PassDownsampling4x4(GLuint texture, FrameBuffer* fb);
    void PassKawaseBlur(GLuint texture, FrameBuffer* fb, int iteration);
    void PassApply(GLuint texture, FrameBuffer* fb = nullptr);

private:
    std::unique_ptr<SDFText> text;
    std::unique_ptr<FullscreenPassGeometry> fs_pass_geom;
    GLuint sampler;
    GLuint texture;
    ShaderPipeline pipeline_fullscreen_quad;
    ShaderPipeline pipeline_high_luminance_region_extraction;
    ShaderPipeline pipeline_downsampling_2x2;
    ShaderPipeline pipeline_downsampling_4x4;
    ShaderPipeline pipeline_kawase_blur;
    ShaderPipeline pipeline_apply;

    std::unique_ptr<FrameBuffer> scene_rt;
    std::unique_ptr<FrameBuffer> high_luminance_region_rt;
    std::vector<std::array<std::unique_ptr<FrameBuffer>, 3>> downsampled_rts;

    std::string shader_kernel_name;

    bool is_filter_enabled;
};