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

    void DrawFullScreenQuad(GLuint texture);

    void PassHighLuminanceRegionExtraction(FrameBuffer* input, FrameBuffer* output);
    void PassDownsampling2x2(FrameBuffer* input, FrameBuffer* output);
    void PassDownsampling4x4(FrameBuffer* input, FrameBuffer* output);
    void PassKawaseBlur(FrameBuffer* input, FrameBuffer* output, int iteration);
    void PassBloom(FrameBuffer* input, FrameBuffer* output);
    void PassStreak(FrameBuffer* input, FrameBuffer* output);
    void PassStreak(FrameBuffer* input, FrameBuffer* output, float dx, float dy, float attenuation, int pass);
    void PassTonemapping(FrameBuffer* input, FrameBuffer* output = nullptr);
    void PassApply(FrameBuffer* input, FrameBuffer* output = nullptr);

private:
    std::unique_ptr<SDFText> text;
    std::unique_ptr<FullscreenPassGeometry> fs_pass_geom;
    GLuint sampler;
    std::vector<GLuint> textures;
    int selected_texture_index;

    ShaderPipeline pipeline_fullscreen_quad;
    ShaderPipeline pipeline_high_luminance_region_extraction;
    ShaderPipeline pipeline_downsampling_2x2;
    ShaderPipeline pipeline_downsampling_4x4;
    ShaderPipeline pipeline_kawase_blur;
    ShaderPipeline pipeline_streak;
    ShaderPipeline pipeline_tonemapping;
    ShaderPipeline pipeline_apply;

    std::unique_ptr<FrameBuffer> scene_rt;
    std::unique_ptr<FrameBuffer> debug_rt;
    std::unique_ptr<FrameBuffer> high_luminance_region_rt;
    std::vector<std::array<std::unique_ptr<FrameBuffer>, 3>> bloom_rts;
    std::array<std::unique_ptr<FrameBuffer>, 3> streak_rts;

    float exposure;
    bool is_bloom_enabled;
    bool is_streak_enabled;
    bool is_debug_enabled;
};