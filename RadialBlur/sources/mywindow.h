#pragma once
#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>
#include <array>
#include <tuple>
#include <hasenpfote/math/vector2.h>
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
    void DrawTextLines(std::vector<std::string> text_lines);

    void DrawFullScreenQuad(GLuint texture);
    void PassHighLuminanceRegionExtraction(FrameBuffer* input, FrameBuffer* output);
    void PassSimpleRadialBlur(FrameBuffer* input, FrameBuffer* output);
    void PassSimpleRadialBlur(FrameBuffer* input, FrameBuffer* output, float x, float y, float attenuation);
    void PassCustomRadialBlur(FrameBuffer* input, FrameBuffer* output);
    void PassCustomRadialBlur(FrameBuffer* input, FrameBuffer* output, float x, float y, float attenuation, int pass, int num_of_passes);
    void PassApply(FrameBuffer* input, FrameBuffer* output = nullptr);

private:
    std::unique_ptr<SDFText> text;
    std::unique_ptr<FullscreenPassGeometry> fs_pass_geom;
    GLuint nearest_sampler;
    GLuint linear_sampler;

    std::vector<std::tuple<GLuint, std::filesystem::path>> selectable_textures;
    int selected_texture_index;

    ShaderPipeline pipeline_fullscreen_quad;
    ShaderPipeline pipeline_high_luminance_region_extraction;
    ShaderPipeline pipeline_simple_radial_blur;
    ShaderPipeline pipeline_custom_radial_blur;
    ShaderPipeline pipeline_apply;

    std::unique_ptr<FrameBuffer> scene_rt;
    std::unique_ptr<FrameBuffer> debug_rt;
    std::unique_ptr<FrameBuffer> high_luminance_region_rt;
    std::array<std::unique_ptr<FrameBuffer>, 3> radial_blur_rts;

    hasenpfote::math::Vector2 mouse_pos;
    hasenpfote::math::Vector2 origin_texcoord;

    bool is_debug_enabled;
    bool is_filter_enabled;
    int radial_blur_mode;
    float attn_coef;    // attenuation coefficient
};