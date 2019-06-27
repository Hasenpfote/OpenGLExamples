#pragma once
#include <iostream>
#include <hasenpfote/math/quaternion.h>
#include "../../Common/window.h"
#include "../../Common/sdf_text.h"
#include "fullscreen_pass_geometry.h"
#include "framebuffer.h"

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
    void PassDownsample(GLuint texture, FrameBuffer* fb);
    void PassKawaseBlur(GLuint texture, FrameBuffer* fb, int iteration);
    void PassApply(GLuint texture);

private:
    std::unique_ptr<SDFText> text;
    FullscreenPassGeometry fsp_geom;
    GLuint sampler;
    GLuint texture;
    ShaderPipeline pipeline_fullscreen_quad;
    ShaderPipeline pipeline_apply;
    ShaderPipeline pipeline_downsample_2x2;
    ShaderPipeline pipeline_downsample_4x4;
    ShaderPipeline pipeline_kawase_blur_filter;

    std::unique_ptr<FrameBuffer> scene_rt;
    std::unique_ptr<FrameBuffer> ds_rt_0;
    std::unique_ptr<FrameBuffer> ds_rt_1;

    std::string shader_kernel_name;

    bool is_filter_enabled;
};