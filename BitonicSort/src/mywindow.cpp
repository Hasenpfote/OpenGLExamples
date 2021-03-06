﻿#include <iomanip>
#include <sstream>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../../common/logger.h"
#include "bitonic_sort.h"
#include "mywindow.h"

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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0);

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

    fs_quad = std::make_unique<FullScreenQuad>();

    glGenSamplers(1, &nearest_sampler);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(nearest_sampler, GL_TEXTURE_LOD_BIAS, 0.0f);

    glGenSamplers(1, &linear_sampler);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(linear_sampler, GL_TEXTURE_LOD_BIAS, 0.0f);

    RecreateResources(width, height);

    auto& rm = System::GetMutableInstance().GetResourceManager();

    pipeline_noise = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/noise.vs"),
            rm.GetResource<Program>("assets/shaders/noise.fs")})
            );

    pipeline_decode = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/decode.vs"),
            rm.GetResource<Program>("assets/shaders/decode.fs")})
            );

    pipeline_sort = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/sort.vs"),
            rm.GetResource<Program>("assets/shaders/sort.fs")})
            );

    pipeline_apply = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/apply.vs"),
            rm.GetResource<Program>("assets/shaders/apply.fs")})
            );

    state = State::Idle;
}

void MyWindow::Cleanup()
{
    if(glIsSampler(nearest_sampler))
        glDeleteSamplers(1, &nearest_sampler);
    if(glIsSampler(linear_sampler))
        glDeleteSamplers(1, &linear_sampler);
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
    //
    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if(state == State::Stopped)
        {
            state = State::Idle;
        }
        else if(state == State::Idle)
        {
            state = State::Ready;
        }
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

    state = State::Idle;
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

    // 1) sort.
    PassSort(output_rt.get());
    // 2) result.
    glEnable(GL_FRAMEBUFFER_SRGB);
    PassApply(output_rt.get());
    glDisable(GL_FRAMEBUFFER_SRGB);

    // Display debug information.
    {
        std::ostringstream oss;
        //std::streamsize ss = std::cout.precision();

        oss << std::fixed << std::setprecision(2);

        oss << "FPS:UPS=" << GetFPS() << ":" << GetUPS();
        oss << "\n";

        oss << "Screen size: " << width << "x" << height;
        oss << "\n";

        GLboolean ms;
        glGetBooleanv(GL_MULTISAMPLE, &ms);
        oss << "MultiSample:" << ((ms == GL_TRUE) ? "On" : "Off") << "(Toggle MultiSample: m)";
        oss << "\n";

        oss << "State:";
        if(state == State::Idle)
            oss << "Idle (Change the state with the space key)";
        else if(state == State::Ready)
            oss << "Ready";
        else if(state == State::Working)
            oss << "Working";
        else if(state == State::Stopped)
            oss << "Stopped (Change the state with the space key)";
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
        const auto name = std::string("input_rt_color");
        input_rt = recreate_fb(name, 1, GL_RGBA16F, width, height);
    }
    {
        auto width2 = static_cast<decltype(width)>(bitonic_sort::next_higher_power_of_two(width));
        auto height2 = static_cast<decltype(height)>(bitonic_sort::next_higher_power_of_two(height));

        std::stringstream ss;

        ss << "sort_rt_color_" << 0;
        auto sort_rt_0 = recreate_fb(ss.str(), 1, GL_RGBA16F, width2, height2);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        ss << "sort_rt_color_" << 1;
        auto sort_rt_1 = recreate_fb(ss.str(), 1, GL_RGBA16F, width2, height2);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        sort_rts = { std::move(sort_rt_0), std::move(sort_rt_1) };
    }
    {
        const auto name = std::string("output_rt_color");
        output_rt = recreate_fb(name, 1, GL_RGBA16F, width, height);
    }
}

void MyWindow::PassNoise(FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        pipeline_noise->GetPipelineUniform().Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

        pipeline_noise->Bind();
        {
            fs_quad->Draw();
        }
        pipeline_noise->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassEncode(FrameBuffer* input, FrameBuffer* output)
{
    GLint viewport[4];

    input->Bind();
    glGetIntegerv(GL_VIEWPORT, viewport);
    auto i_width = viewport[2];
    auto i_height = viewport[3];
    input->Unbind();

    output->Bind();
    {
        GLfloat clear_color[] = { 1.0f, 0.0, 0.0f, 1.0f };
        glClearTexImage(output->GetColorTexture(), 0, GL_RGBA, GL_FLOAT, &clear_color);

        glViewport(0, 0, i_width, i_height);

        auto& uniform = pipeline_apply->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(i_width), 1.0f / static_cast<float>(i_height)));

        pipeline_apply->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_apply->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassDecode(FrameBuffer* input, FrameBuffer* output)
{
    GLint viewport[4];

    input->Bind();
    glGetIntegerv(GL_VIEWPORT, viewport);
    auto i_width = viewport[2];
    auto i_height = viewport[3];
    input->Unbind();

    output->Bind();
    {
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_decode->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_src_resolution", glm::vec2(static_cast<float>(i_width), static_cast<float>(i_height)));
        uniform.Set("u_dst_resolution", glm::vec2(static_cast<float>(viewport[2]), static_cast<float>(viewport[3])));

        pipeline_decode->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, nearest_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_decode->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassSort(FrameBuffer* output)
{
    if(state == State::Idle)
    {
        PassNoise(input_rt.get());
        PassApply(input_rt.get(), output);
    }
    else if(state == State::Ready)
    {
        GLint viewport[4];
        sort_rts[0]->Bind();
        glGetIntegerv(GL_VIEWPORT, viewport);
        sort_rts[0]->Unbind();

        auto N = viewport[2] * viewport[3];
        num_passes = bitonic_sort::get_num_passes(N);
        pass = 1;

        PassEncode(input_rt.get(), sort_rts[0].get());
        PassDecode(sort_rts[0].get(), output);
        state = State::Working;
    }
    else if(state == State::Working)
    {
        if(pass <= num_passes)
        {
            auto params = bitonic_sort::get_params(pass);
            auto current = (pass - 1) % 2;
            auto src = sort_rts[current].get();
            auto dst = sort_rts[1 - current].get();

            PassSort(src, dst, std::get<2>(params), std::get<3>(params), std::get<4>(params));
            PassDecode(dst, output);

            pass++;
        }
        else
        {
            state = State::Stopped;
        }
    }
}

void MyWindow::PassSort(FrameBuffer* input, FrameBuffer* output, int seq_size, int offset, int range)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_sort->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_resolution", glm::vec2(static_cast<float>(viewport[2]), static_cast<float>(viewport[3])));
        uniform.Set("u_seq_size", static_cast<float>(seq_size));
        uniform.Set("u_offset", static_cast<float>(offset));
        uniform.Set("u_range", static_cast<float>(range));

        pipeline_sort->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, nearest_sampler);

            fs_quad->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_sort->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassApply(FrameBuffer* input, FrameBuffer* output)
{
    if(output != nullptr)
        output->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    auto& uniform = pipeline_apply->GetPipelineUniform();
    uniform.Set("u_tex0", 0);
    uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

    pipeline_apply->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, linear_sampler);

        fs_quad->Draw();

        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_apply->Unbind();

    if(output != nullptr)
        output->Unbind();
}