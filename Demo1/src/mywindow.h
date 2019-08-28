#pragma once
#include <memory>
#include <filesystem>
#include <vector>
#include <array>
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
    void OnGUI() override;

    void RecreateResources(int width, int height);
    void DrawTextLines(const std::vector<std::string>& text_lines);

    void DrawFullScreenQuad(GLuint texture);

    float ComputeAverageLuminance(FrameBuffer* input);
    void PassLogLuminance(FrameBuffer* input, FrameBuffer* output);
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
    GLuint nearest_sampler;
    GLuint linear_sampler;

    std::vector<std::filesystem::path> selectable_textures;
    int selected_texture_index;

    std::unique_ptr<ProgramPipeline> pipeline_fullscreen_quad;
    std::unique_ptr<ProgramPipeline> pipeline_log_luminance;
    std::unique_ptr<ProgramPipeline> pipeline_high_luminance_region_extraction;
    std::unique_ptr<ProgramPipeline> pipeline_downsampling_2x2;
    std::unique_ptr<ProgramPipeline> pipeline_downsampling_4x4;
    std::unique_ptr<ProgramPipeline> pipeline_kawase_blur;
    std::unique_ptr<ProgramPipeline> pipeline_streak;
    std::unique_ptr<ProgramPipeline> pipeline_tonemapping;
    std::unique_ptr<ProgramPipeline> pipeline_apply;

    std::unique_ptr<FrameBuffer> scene_rt;
    std::unique_ptr<FrameBuffer> luminance_rt;
    std::unique_ptr<FrameBuffer> debug_rt;
    std::unique_ptr<FrameBuffer> high_luminance_region_rt;
    std::vector<std::array<std::unique_ptr<FrameBuffer>, 3>> bloom_rts;
    std::array<std::unique_ptr<FrameBuffer>, 3> streak_rts;

    float exposure;
    float lum_soft_threshold;
    float lum_hard_threshold;
    float average_luminance;
    bool is_bloom_enabled;
    bool is_streak_enabled;
    bool is_debug_enabled;
    bool is_tonemapping_enabled;
    bool is_auto_exposure_enabled;
};