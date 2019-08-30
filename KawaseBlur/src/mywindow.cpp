﻿#include <iomanip>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../../common/logger.h"
#include "mywindow.h"

const std::unordered_map <std::string, std::vector<int>> shader_kernel =
{
    // Matches a NxN Gaussian blur kernel.
    {"gaussian_7x7",        {0, 0}},
    {"gaussian_15x15",      {0, 1, 1}},
    {"gaussian_23x23",      {0, 1, 1, 2}},
    {"gaussian_35x35",      {0, 1, 2, 2, 3}},
    {"gaussian_63x63",      {0, 1, 2, 3, 4, 4, 5}},
    {"gaussian_127x127",    {0, 1, 2, 3, 4, 5, 7, 8, 9, 10}},
};

MyWindow::MyWindow()
{
    LOG_D(__func__);
}

MyWindow::~MyWindow()
{
    LOG_D(__func__);
}

void MyWindow::Setup()
{
    //
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glDisable(GL_BLEND);
    //glDisable(GL_LIGHTING);

    glDisable(GL_MULTISAMPLE);

    glClearColor(160.0f / 255.0f, 216.0f / 255.0f, 239.0f / 255.0f, 1.0);

    //
    auto window = GetWindow();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    //
    {
        auto& camera = System::GetMutableInstance().GetCamera();

        auto& vp = camera.viewport();
        vp.origin() = glm::vec2(0, 0);
        vp.size() = glm::vec2(width, height);
        vp.depth_range() = glm::vec2(1.0f, 10000.0f);

        camera.Update(0.0);
    }
    // load shader.
    {
        std::filesystem::path dirpath("assets/shaders");
        auto& rm = System::GetMutableInstance().GetResourceManager();
        rm.AddResourcesFromDirectory<Program>(dirpath, false);
    }
    // load texture.
    {
        std::filesystem::path dirpath("assets/textures");
        auto& rm = System::GetMutableInstance().GetResourceManager();
        rm.AddResourcesFromDirectory<Texture>(dirpath, false);
    }
    // generate font.
    {
        std::filesystem::path fontpath = "../common/assets/fonts/test.fnt";
        auto font = std::make_shared<Font>(fontpath);
        text = std::make_unique<SDFText>(font, std::make_shared<SDFTextRenderer>());
        text->SetSmoothness(1.0f);
    }

    auto& rm = System::GetMutableInstance().GetResourceManager();
    //
    texture = rm.GetResource<Texture>("assets/textures/testimg_1920x1080.png")->GetTexture();

    fs_quad = std::make_unique<FullScreenQuad>();

    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_LOD_BIAS, 0.0f);

    RecreateResources(width, height);

    pipeline_fullscreen_quad = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/fullscreen_quad.vs"),
            rm.GetResource<Program>("assets/shaders/fullscreen_quad.fs")})
            );

    pipeline_apply = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/apply.vs"),
            rm.GetResource<Program>("assets/shaders/apply.fs")})
            );

    pipeline_downsampling_2x2 = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/downsampling.vs"),
            rm.GetResource<Program>("assets/shaders/downsampling_2x2.fs")})
            );

    pipeline_downsampling_4x4 = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/downsampling.vs"),
            rm.GetResource<Program>("assets/shaders/downsampling_4x4.fs")})
            );

    pipeline_kawase_blur = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/kawase_blur.vs"),
            rm.GetResource<Program>("assets/shaders/kawase_blur.fs")})
            );

    shader_kernel_name = "gaussian_7x7";

    is_filter_enabled = false;
}

void MyWindow::Cleanup()
{
    if(glIsSampler(sampler))
        glDeleteSamplers(1, &sampler);
}

void MyWindow::OnKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window::OnKey(window, key, scancode, action, mods);

    System::GetMutableInstance().GetCamera().OnKey(key, scancode, action, mods);

    if(key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        GLboolean ms;
        glGetBooleanv(GL_MULTISAMPLE, &ms);
        if(ms == GL_TRUE)
        {
            glDisable(GL_MULTISAMPLE);
        }
        else
        {
            glEnable(GL_MULTISAMPLE);
        }
    }
    if(key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        is_filter_enabled = !is_filter_enabled;
    }
    if(key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            shader_kernel_name = "gaussian_7x7";
    }
    if(key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            shader_kernel_name = "gaussian_15x15";
    }
    if(key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            shader_kernel_name = "gaussian_23x23";
    }
    if(key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            shader_kernel_name = "gaussian_35x35";
    }
    if(key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            shader_kernel_name = "gaussian_63x63";
    }
    if(key == GLFW_KEY_6 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            shader_kernel_name = "gaussian_127x127";
    }
}

void MyWindow::OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    System::GetMutableInstance().GetCamera().OnMouseMove(xpos, ypos);
}

void MyWindow::OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    System::GetMutableInstance().GetCamera().OnMouseButton(button, action, mods);
}

void MyWindow::OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
{
    System::GetMutableInstance().GetCamera().OnMouseWheel(xoffset, yoffset);
}

void MyWindow::OnResizeFramebuffer(GLFWwindow* window, int width, int height)
{
    if(HasIconified())
        return;
    // re-create
    RecreateResources(width, height);
}

void MyWindow::OnResizeWindow(GLFWwindow* window, int width, int height)
{
    if(HasIconified())
        return;

    auto& camera = System::GetMutableInstance().GetCamera();
    camera.viewport().size() = glm::vec2(width, height);
}

void MyWindow::OnUpdate(double dt)
{
    System::GetMutableInstance().GetCamera().Update(dt);
}

void MyWindow::OnRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto& camera = System::GetConstInstance().GetCamera();

    auto& vp = camera.viewport();
    auto& resolution = camera.viewport().size();
    const auto width = static_cast<int>(resolution.x);
    const auto height = static_cast<int>(resolution.y);
    glViewport(0, 0, width, height);
    //

    // 1) シーンをテクスチャへ描画
    scene_rt->Bind();
    DrawFullScreenQuad();
    scene_rt->Unbind();

    // 2) 1/4 x 1/4 ダウンサンプルを行う
    auto last_blur_rt = ds_rt_0.get();
    PassDownsampling(scene_rt.get(), last_blur_rt);

    // 3) Kawase blur
    if(is_filter_enabled)
    {
        auto it = shader_kernel.find(shader_kernel_name);
        if(it != shader_kernel.cend())
        {
            auto kernel = it->second;
            const auto num_of_passes = kernel.size();
            for(auto i = 0; i < num_of_passes; i++)
            {
                FrameBuffer* src_blur_rt = last_blur_rt;
                FrameBuffer* dst_blur_rt = ((i % 2) == 0) ? ds_rt_1.get() : ds_rt_0.get();
                PassKawaseBlur(src_blur_rt, dst_blur_rt, kernel[i]);
                last_blur_rt = dst_blur_rt;
            }
        }
    }
    // 4) 結果を表示
    glEnable(GL_FRAMEBUFFER_SRGB);
    PassApply(last_blur_rt);
    glDisable(GL_FRAMEBUFFER_SRGB);

    // Display debug information.
    {
        std::ostringstream oss;
        //std::streamsize ss = std::cout.precision();

        oss << std::fixed << std::setprecision(2);

        oss << "FPS:UPS=" << GetFPS() << ":" << GetUPS();
        oss << "\n";

        oss << "Screen size:" << width << "x" << height;
        oss << "\n";

        GLboolean ms;
        glGetBooleanv(GL_MULTISAMPLE, &ms);
        oss << "MultiSample:" << ((ms == GL_TRUE) ? "On" : "Off") << "(Toggle MultiSample: m)";
        oss << "\n";

        oss << "Kawase blur:" << ((is_filter_enabled == GL_TRUE) ? "On" : "Off") << "(Toggle Kawase blur: b)" << " " << shader_kernel_name;
        oss << "\n";

        text->BeginRendering();
        {
            text->DrawString(std::move(oss.str()), 0.0f, 0.0f, 0.5f);
        }
        text->EndRendering();

        //oss.str("");
        //oss.clear(std::stringstream::goodbit);
    }
}

void MyWindow::RecreateResources(int width, int height)
{
    auto recreate_fb = [](const std::string& name, GLsizei levels, GLenum internal_format, GLsizei width, GLsizei height)
    {
        auto& rm = System::GetMutableInstance().GetResourceManager();

        rm.RemoveResource<Texture>(name);

        if(levels == 0)
            levels = Texture::CalcNumOfMipmapLevels(width, height);

        auto p = std::make_unique<Texture>(levels, internal_format, width, height);
        auto texture = p->GetTexture();

        rm.AddResource<Texture>(name, std::move(p));

        return std::make_unique<FrameBuffer>(texture, 0, 0);
    };

    {
        const auto name = std::string("scene_rt_color");
        scene_rt = recreate_fb(name, 1, GL_RGBA16F, width, height);
    }
    {
        auto ds_width = width / 4;
        auto ds_height = height / 4;
        ds_rt_0 = recreate_fb("ds_rt_0_color", 1, GL_RGBA16F, ds_width, ds_height);
        ds_rt_1 = recreate_fb("ds_rt_1_color", 1, GL_RGBA16F, ds_width, ds_height);
    }
}

void MyWindow::DrawFullScreenQuad()
{
    pipeline_fullscreen_quad->GetPipelineUniform().Set("texture0", 0);

    pipeline_fullscreen_quad->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindSampler(0, sampler);

        fs_quad->Draw();

        glActiveTexture(GL_TEXTURE0);
    }
    pipeline_fullscreen_quad->Unbind();
}

void MyWindow::PassDownsampling(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_downsampling_4x4->GetPipelineUniform();
        uniform.Set("texture0", 0);
        uniform.Set("pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

        pipeline_downsampling_4x4->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_quad->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_downsampling_4x4->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassKawaseBlur(FrameBuffer* input, FrameBuffer* output, int iteration)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_kawase_blur->GetPipelineUniform();
        uniform.Set("texture0", 0);
        uniform.Set("params", glm::vec3(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]), static_cast<float>(iteration)));

        pipeline_kawase_blur->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_quad->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_kawase_blur->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassApply(FrameBuffer* input, FrameBuffer* output)
{
    if(output)
        output->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    auto& uniform = pipeline_apply->GetPipelineUniform();
    uniform.Set("texture0", 0);
    uniform.Set("pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

    pipeline_apply->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, sampler);

        fs_quad->Draw();

        glActiveTexture(GL_TEXTURE0);
    }
    pipeline_apply->Unbind();

    if(output)
        output->Unbind();
}