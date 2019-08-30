#include <iomanip>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../../common/logger.h"
#include "mywindow.h"

const std::unordered_map<std::string, std::array<int, 2>> streak_filter =
{
    {"4streaks × 2passes", {4, 2}},
    {"4streaks × 3passes", {4, 3}},
    {"5streaks × 2passes", {5, 2}},
    {"5streaks × 3passes", {5, 3}},
    {"6streaks × 2passes", {6, 2}},
    {"6streaks × 3passes", {6, 3}},
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

    pipeline_streak = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/streak.vs"),
            rm.GetResource<Program>("assets/shaders/streak.fs")})
            );

    pipeline_high_luminance_region_extraction = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/high_luminance_region_extraction.vs"),
            rm.GetResource<Program>("assets/shaders/high_luminance_region_extraction.fs")})
            );

    streak_filter_name = "4streaks × 2passes";

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
            streak_filter_name = "4streaks × 2passes";
    }
    if(key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            streak_filter_name = "4streaks × 3passes";
    }
    if(key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            streak_filter_name = "5streaks × 2passes";
    }
    if(key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            streak_filter_name = "5streaks × 3passes";
    }
    if(key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            streak_filter_name = "6streaks × 2passes";
    }
    if(key == GLFW_KEY_6 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            streak_filter_name = "6streaks × 3passes";
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
    auto& camera = System::GetMutableInstance().GetCamera();
    camera.Update(dt);
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

    // 2) 高輝度領域の抽出(兼ダウンサンプリング)
    PassHighLuminanceRegionExtraction(scene_rt.get(), high_luminance_region_rt.get());

    // 3) ブラーを掛ける前の入力画像
    {
        auto src_rt = high_luminance_region_rt.get();
        auto dst_rt = input_rt.get();
        PassDownsampling2x2(src_rt, dst_rt);
    }

    // 4) Streak の適用
    if(is_filter_enabled)
        PassStreak(input_rt.get(), scene_rt.get());

    glEnable(GL_FRAMEBUFFER_SRGB);
    PassApply(scene_rt.get());
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

        oss << "Streak:" << ((is_filter_enabled == GL_TRUE) ? "On" : "Off") << "(Toggle Streak: b)" << " " << streak_filter_name;
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
        auto ds_width = width / 2;
        auto ds_height = height / 2;

        const auto name = std::string("luminance_rt_color");
        high_luminance_region_rt = recreate_fb(name, 1, GL_RGBA16F, ds_width, ds_height);
    }
    {
        auto ds_width = width / 4;
        auto ds_height = height / 4;

        input_rt = recreate_fb("input_rt_color", 1, GL_RGBA16F, ds_width, ds_height);
        output_rt = recreate_fb("output_rt_color", 1, GL_RGBA16F, ds_width, ds_height);

        work_rts[0] = recreate_fb("work_rts_color_0", 1, GL_RGBA16F, ds_width, ds_height);
        work_rts[1] = recreate_fb("work_rts_color_1", 1, GL_RGBA16F, ds_width, ds_height);
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

void MyWindow::PassHighLuminanceRegionExtraction(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_high_luminance_region_extraction->GetPipelineUniform();
        uniform.Set("texture0", 0);
        uniform.Set("pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

        pipeline_high_luminance_region_extraction->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_quad->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_high_luminance_region_extraction->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassDownsampling2x2(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_downsampling_2x2->GetPipelineUniform();
        uniform.Set("texture0", 0);
        uniform.Set("pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));

        pipeline_downsampling_2x2->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_quad->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_downsampling_2x2->Unbind();
    }
    output->Unbind();
}

void MyWindow::PassDownsampling4x4(FrameBuffer* input, FrameBuffer* output)
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

void MyWindow::PassStreak(FrameBuffer* input, FrameBuffer* output)
{
    // 1) 方向毎にピンポンブラー
    {
        auto it = streak_filter.find(streak_filter_name);
        assert(it != streak_filter.cend());
        auto params = it->second;
        const auto num_of_streaks = params[0];
        const auto num_of_passes = params[1];
        const auto additional_angle = 360.0f / static_cast<float>(num_of_streaks);

        GLfloat clear_color[] = { 0.0f, 0.0, 0.0f, 1.0f };
        glClearTexImage(output_rt->GetColorTexture(), 0, GL_RGBA, GL_FLOAT, &clear_color);

        float angle = 45.0f;
        for(auto i = 0; i < num_of_streaks; i++)
        {
            auto theta = glm::radians(angle);
            auto dx = std::cos(theta);
            auto dy = std::sin(theta);

            auto last_rt = input_rt.get();
            for(auto j = 0; j < num_of_passes; j++)
            {
                FrameBuffer* src_rt = last_rt;
                FrameBuffer* dst_rt = ((j % 2) == 0) ? work_rts[0].get() : work_rts[1].get();
                PassStreak(src_rt, dst_rt, dx, dy, 0.95f, j);
                last_rt = dst_rt;
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            PassApply(last_rt, output_rt.get());
            glDisable(GL_BLEND);

            angle += additional_angle;
        }
    }
    // 2) 合成
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        PassApply(output_rt.get(), output);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void MyWindow::PassStreak(FrameBuffer* input, FrameBuffer* output, float dx, float dy, float attenuation, int pass)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        auto& uniform = pipeline_streak->GetPipelineUniform();
        uniform.Set("u_tex0", 0);
        uniform.Set("u_pixel_size", glm::vec2(1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3])));
        uniform.Set("u_direction", glm::vec2(dx, dy));
        uniform.Set("u_params", glm::vec2(attenuation, static_cast<float>(pass)));

        pipeline_streak->Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_quad->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_streak->Unbind();
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

    if(output != nullptr)
        output->Unbind();
}