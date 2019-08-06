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

    void PassNoise(FrameBuffer* output);
    void PassEncode(FrameBuffer* input, FrameBuffer* output);
    void PassDecode(FrameBuffer* input, FrameBuffer* output);
    void PassSort(FrameBuffer* output);
    void PassSort(FrameBuffer* input, FrameBuffer* output, int seq_size, int offset, int range);
    void PassApply(FrameBuffer* input, FrameBuffer* output = nullptr);

private:
    std::unique_ptr<SDFText> text;
    std::unique_ptr<FullscreenPassGeometry> fs_pass_geom;
    GLuint nearest_sampler;
    GLuint linear_sampler;

    common::ShaderPipeline pipeline_noise;
    common::ShaderPipeline pipeline_decode;
    common::ShaderPipeline pipeline_sort;
    common::ShaderPipeline pipeline_apply;

    std::unique_ptr<FrameBuffer> input_rt;
    std::array<std::unique_ptr<FrameBuffer>, 2> sort_rts;
    std::unique_ptr<FrameBuffer> output_rt;

    enum class State
    {
        Idle,
        Ready,
        Working,
        Stopped
    };

    State state;
    int num_passes;
    int pass;
};