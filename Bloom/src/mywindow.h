#pragma once
#include <iostream>
#include <array>
#include <vector>
#include "../../common/window.h"
#include "../../common/system.h"
#include "../../common/render/texture.h"
#include "../../common/render/framebuffer.h"
#include "../../common/render/shader/shader.h"
#include "../../common/render/text/sdf_text.h"
#include "fullscreen_pass_geometry.h"

class MyWindow final : public common::Window
{
    using System = common::System;
    using Texture = common::render::Texture;
    using Program = common::render::shader::Program;
    using ProgramPipeline = common::render::shader::ProgramPipeline;
    using FrameBuffer = common::render::FrameBuffer;
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

    void RecreateResources(int width, int height);

    void DrawFullScreenQuad();
    void PassHighLuminanceRegionExtraction(FrameBuffer* input, FrameBuffer* output);
    void PassDownsampling2x2(FrameBuffer* input, FrameBuffer* output);
    void PassDownsampling4x4(FrameBuffer* input, FrameBuffer* output);
    void PassKawaseBlur(FrameBuffer* input, FrameBuffer* output, int iteration);
    void PassBloom(FrameBuffer* input, FrameBuffer* output);
    void PassApply(FrameBuffer* input, FrameBuffer* output = nullptr);

private:
    std::unique_ptr<SDFText> text;
    std::unique_ptr<FullscreenPassGeometry> fs_pass_geom;
    GLuint sampler;
    GLuint texture;

    std::unique_ptr<ProgramPipeline> pipeline_fullscreen_quad;
    std::unique_ptr<ProgramPipeline> pipeline_high_luminance_region_extraction;
    std::unique_ptr<ProgramPipeline> pipeline_downsampling_2x2;
    std::unique_ptr<ProgramPipeline> pipeline_downsampling_4x4;
    std::unique_ptr<ProgramPipeline> pipeline_kawase_blur;
    std::unique_ptr<ProgramPipeline> pipeline_apply;

    std::unique_ptr<FrameBuffer> scene_rt;
    std::unique_ptr<FrameBuffer> high_luminance_region_rt;
    std::vector<std::array<std::unique_ptr<FrameBuffer>, 3>> downsampled_rts;

    std::string shader_kernel_name;

    bool is_filter_enabled;
};