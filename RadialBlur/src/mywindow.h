#pragma once
#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>
#include <array>
#include <tuple>
#include "../../common/window.h"
#include "../../common/system.h"
#include "../../common/render/texture.h"
#include "../../common/render/framebuffer.h"
#include "../../common/render/fullscreen_quad.h"
#include "../../common/render/shader/shader.h"
#include "../../common/render/text/sdf_text.h"

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
    using FullScreenQuad = common::render::FullScreenQuad;

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
    void PassSimpleRadialBlur(FrameBuffer* input, FrameBuffer* output);
    void PassSimpleRadialBlur(FrameBuffer* input, FrameBuffer* output, float x, float y, float attenuation);
    void PassCustomRadialBlur(FrameBuffer* input, FrameBuffer* output);
    void PassCustomRadialBlur(FrameBuffer* input, FrameBuffer* output, float x, float y, float attenuation, int pass, int num_of_passes);
    void PassApply(FrameBuffer* input, FrameBuffer* output = nullptr);

private:
    std::unique_ptr<SDFText> text;
    std::unique_ptr<FullScreenQuad> fs_quad;
    GLuint nearest_sampler;
    GLuint linear_sampler;

    std::vector<std::tuple<GLuint, std::filesystem::path>> selectable_textures;
    int selected_texture_index;

    std::unique_ptr<ProgramPipeline> pipeline_fullscreen_quad;
    std::unique_ptr<ProgramPipeline> pipeline_high_luminance_region_extraction;
    std::unique_ptr<ProgramPipeline> pipeline_simple_radial_blur;
    std::unique_ptr<ProgramPipeline> pipeline_custom_radial_blur;
    std::unique_ptr<ProgramPipeline> pipeline_apply;

    std::unique_ptr<FrameBuffer> scene_rt;
    std::unique_ptr<FrameBuffer> debug_rt;
    std::unique_ptr<FrameBuffer> high_luminance_region_rt;
    std::array<std::unique_ptr<FrameBuffer>, 3> radial_blur_rts;

    int mouse_x, mouse_y;

    bool is_debug_enabled;
    bool is_filter_enabled;
    int radial_blur_mode;
    float attn_coef;    // attenuation coefficient
};