#pragma once
#include <iostream>
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

    void DrawFullScreenQuad();
    void PassDownsampling(FrameBuffer* input, FrameBuffer* output);
    void PassKawaseBlur(FrameBuffer* input, FrameBuffer* output, int iteration);
    void PassApply(FrameBuffer* input, FrameBuffer* output = nullptr);

private:
    std::unique_ptr<SDFText> text;
    std::unique_ptr<FullscreenPassGeometry> fs_pass_geom;
    GLuint sampler;
    GLuint texture;
    common::ShaderPipeline pipeline_fullscreen_quad;
    common::ShaderPipeline pipeline_downsampling_2x2;
    common::ShaderPipeline pipeline_downsampling_4x4;
    common::ShaderPipeline pipeline_kawase_blur;
    common::ShaderPipeline pipeline_apply;

    std::unique_ptr<FrameBuffer> scene_rt;
    std::unique_ptr<FrameBuffer> ds_rt_0;
    std::unique_ptr<FrameBuffer> ds_rt_1;

    std::string shader_kernel_name;

    bool is_filter_enabled;
};