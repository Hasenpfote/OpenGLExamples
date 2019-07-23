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

    std::vector<std::tuple<GLuint, std::filesystem::path>> selectable_textures;
    int selected_texture_index;

    ShaderPipeline pipeline_fullscreen_quad;
    ShaderPipeline pipeline_log_luminance;
    ShaderPipeline pipeline_high_luminance_region_extraction;
    ShaderPipeline pipeline_downsampling_2x2;
    ShaderPipeline pipeline_downsampling_4x4;
    ShaderPipeline pipeline_kawase_blur;
    ShaderPipeline pipeline_streak;
    ShaderPipeline pipeline_tonemapping;
    ShaderPipeline pipeline_apply;

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