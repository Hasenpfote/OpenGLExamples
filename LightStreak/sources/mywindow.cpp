﻿#include <iomanip>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <hasenpfote/assert.h>
//#include <hasenpfote//math/utility.h>
#include <hasenpfote//math/utils.h>
#include <hasenpfote/math/vector3.h>
#include <hasenpfote/math/vector4.h>
#include <hasenpfote/math/cmatrix4.h>
#include <hasenpfote/math/axis_angle.h>
#include "../../Common/system.h"
#include "../../Common/logger.h"
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
    using namespace hasenpfote::math;
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

    auto& camera = System::GetMutableInstance().GetCamera();

    camera.SetViewport(0, 0, width, height);
    //camera.Set35mmEquivalentFocalLength(50.0f);
    camera.SetClippingPlane(1.0f, 10000.0f);

    camera.SetPosition(Vector3(500.0f, 1000.0f, 1500.0f));
    camera.SetTargetPosition(Vector3(500.0f, 0.0f, 500.0f));

    camera.Update(0.0f);

    // load shader.
    {
        auto& man = System::GetMutableInstance().GetShaderManager();
        std::filesystem::path directory("assets/shaders");
        man.LoadShaderPrograms(directory);
    }

    // load texture.
    {
        std::filesystem::path directory("assets/textures");
        const std::filesystem::path extension(".png");
        auto& man = System::GetMutableInstance().GetTextureManager();
        man.LoadTextures(directory, extension);
    }
    // generate font.
    {
        std::filesystem::path fontpath = "../Common/assets/fonts/test.fnt";
        auto font = std::make_shared<text::Font>(fontpath);
        text = std::make_unique<SDFText>(font, std::make_shared<SDFTextRenderer>());
        text->SetSmoothness(1.0f);
    }
    //
    texture = System::GetConstInstance().GetTextureManager().GetTexture("assets/textures/testimg_1920x1080.png");

    fs_pass_geom = std::make_unique<FullscreenPassGeometry>();

    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_LOD_BIAS, 0.0f);

    RecreateResources(width, height);

    auto& man = System::GetConstInstance().GetShaderManager();
    pipeline_fullscreen_quad.Create();
    pipeline_fullscreen_quad.SetShaderProgram(man.GetShaderProgram("assets/shaders/fullscreen_quad.vs"));
    pipeline_fullscreen_quad.SetShaderProgram(man.GetShaderProgram("assets/shaders/fullscreen_quad.fs"));

    pipeline_apply.Create();
    pipeline_apply.SetShaderProgram(man.GetShaderProgram("assets/shaders/apply.vs"));
    pipeline_apply.SetShaderProgram(man.GetShaderProgram("assets/shaders/apply.fs"));

    pipeline_downsampling_2x2.Create();
    pipeline_downsampling_2x2.SetShaderProgram(man.GetShaderProgram("assets/shaders/downsampling.vs"));
    pipeline_downsampling_2x2.SetShaderProgram(man.GetShaderProgram("assets/shaders/downsampling_2x2.fs"));

    pipeline_downsampling_4x4.Create();
    pipeline_downsampling_4x4.SetShaderProgram(man.GetShaderProgram("assets/shaders/downsampling.vs"));
    pipeline_downsampling_4x4.SetShaderProgram(man.GetShaderProgram("assets/shaders/downsampling_4x4.fs"));

    pipeline_streak.Create();
    pipeline_streak.SetShaderProgram(man.GetShaderProgram("assets/shaders/streak.vs"));
    pipeline_streak.SetShaderProgram(man.GetShaderProgram("assets/shaders/streak.fs"));

    pipeline_high_luminance_region_extraction.Create();
    pipeline_high_luminance_region_extraction.SetShaderProgram(man.GetShaderProgram("assets/shaders/high_luminance_region_extraction.vs"));
    pipeline_high_luminance_region_extraction.SetShaderProgram(man.GetShaderProgram("assets/shaders/high_luminance_region_extraction.fs"));

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

    auto& camera = System::GetMutableInstance().GetCamera();
    camera.OnKey(key, scancode, action, mods);

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
    System::GetMutableInstance().GetCamera().SetViewportSize(width, height);
}

void MyWindow::OnUpdate(double dt)
{
    auto& camera = System::GetMutableInstance().GetCamera();
    camera.Update(dt);
}

void MyWindow::OnRender()
{
    using namespace hasenpfote::math;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto& camera = System::GetConstInstance().GetCamera();
    auto& vp = camera.GetViewport();

    const auto width = vp.GetWidth();
    const auto height = vp.GetHeight();
    glViewport(0, 0, width, height);
    //

    // 1) シーンをテクスチャへ描画
    scene_rt->Bind();
    DrawFullScreenQuad();
    scene_rt->Unbind();

    // 2) 高輝度領域の抽出(兼ダウンサンプリング)
    PassHighLuminanceRegionExtraction(scene_rt.get()->GetColorTexture(), high_luminance_region_rt.get());

    // 3) ブラーを掛ける前の入力画像
    {
        auto src_rt = high_luminance_region_rt.get();
        auto dst_rt = input_rt.get();
        PassDownsampling2x2(src_rt->GetColorTexture(), dst_rt);
    }

    // 4) 方向毎にピンポンブラー
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
            auto theta = hasenpfote::math::ConvertDegreesToRadians(angle);
            auto dx = std::cos(theta);
            auto dy = std::sin(theta);

            auto last_rt = input_rt.get();
            for(auto j = 0; j < num_of_passes; j++)
            {
                FrameBuffer* src_rt = last_rt;
                FrameBuffer* dst_rt = ((j % 2) == 0) ? work_rts[0].get() : work_rts[1].get();
                PassStreak(src_rt->GetColorTexture(), dst_rt, dx, dy, 0.95f, j);
                last_rt = dst_rt;
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            PassApply(last_rt->GetColorTexture(), output_rt.get());
            glDisable(GL_BLEND);

            angle += additional_angle;
        }
    }

    // 5) 合成
    {
        glDepthMask(GL_FALSE);
        PassApply(scene_rt->GetColorTexture());

        if(is_filter_enabled){
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);

            PassApply(output_rt->GetColorTexture());

            glDisable(GL_BLEND);
        }

        glDepthMask(GL_TRUE);
    }

    // 情報の表示
    auto metrics = text->GetFont().GetFontMetrics();
    auto line_height = static_cast<float>(metrics.GetLineHeight()); 
    const float scale = 1.0f;
    const float fh = line_height * scale;

    static const Vector4 color(1.0f, 1.0f, 1.0f, 1.0f);
    text->SetColor(static_cast<const GLfloat*>(color));

    text->BeginRendering();

    std::ostringstream oss;
    oss << "FPS:UPS=";
    oss << std::fixed << std::setprecision(2);
    oss << GetFPS() << ":" << GetUPS();
    text->DrawString(oss.str(), 0.0f, fh, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Screen size:";
    oss << camera.GetViewport().GetWidth() << "x" << camera.GetViewport().GetHeight();
    text->DrawString(oss.str(), 0.0f, fh * 2.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Aov D=" << ConvertRadiansToDegrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Diagonal));
    oss << " H=" << ConvertRadiansToDegrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Horizontal));
    oss << " V=" << ConvertRadiansToDegrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Vertical));
    text->DrawString(oss.str(), 0.0f, fh * 3.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Focal length=" << camera.GetFocalLength() << " (35mm=" << camera.Get35mmEquivalentFocalLength() << ")";
    text->DrawString(oss.str(), 0.0f, fh * 4.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Zoom=x" << camera.GetZoomMagnification();
    text->DrawString(oss.str(), 0.0f, fh * 5.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    GLboolean ms;
    glGetBooleanv(GL_MULTISAMPLE, &ms);
    oss << "MultiSample:" << ((ms == GL_TRUE) ? "On" : "Off") << "(Toggle MultiSample: m)";
    text->DrawString(oss.str(), 0.0f, fh * 6.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Streak:" << ((is_filter_enabled == GL_TRUE) ? "On" : "Off") << "(Toggle Streak: b)" << " " << streak_filter_name;
    text->DrawString(oss.str(), 0.0f, fh * 7.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    text->EndRendering();
}

void MyWindow::RecreateResources(int width, int height)
{
    GLuint color_texture;

    System::GetMutableInstance().GetTextureManager().DeleteTexture("scene_rt_color");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("scene_rt_color", GL_RGBA8, width, height);
    scene_rt = std::make_unique<FrameBuffer>(color_texture, 0, 0);
    //
    auto ds_width = width / 2;
    auto ds_height = height / 2;

    System::GetMutableInstance().GetTextureManager().DeleteTexture("luminance_rt_color");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("luminance_rt_color", GL_RGBA8, ds_width, ds_height);
    high_luminance_region_rt = std::make_unique<FrameBuffer>(color_texture, 0, 0);
    //
    ds_width /= 2;
    ds_height /= 2;

    System::GetMutableInstance().GetTextureManager().DeleteTexture("input_rt_color");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("input_rt_color", GL_RGBA8, ds_width, ds_height);
    input_rt = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    System::GetMutableInstance().GetTextureManager().DeleteTexture("output_rt_color");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("output_rt_color", GL_RGBA8, ds_width, ds_height);
    output_rt = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    System::GetMutableInstance().GetTextureManager().DeleteTexture("work_rts_color_0");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("work_rts_color_0", GL_RGBA8, ds_width, ds_height);
    work_rts[0] = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    System::GetMutableInstance().GetTextureManager().DeleteTexture("work_rts_color_1");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("work_rts_color_1", GL_RGBA8, ds_width, ds_height);
    work_rts[1] = std::make_unique<FrameBuffer>(color_texture, 0, 0);
}

void MyWindow::DrawFullScreenQuad()
{
    pipeline_fullscreen_quad.SetUniform1i("texture0", 0);

    pipeline_fullscreen_quad.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    fs_pass_geom->Draw();

    glActiveTexture(GL_TEXTURE0);

    pipeline_fullscreen_quad.Unbind();
}

void MyWindow::PassHighLuminanceRegionExtraction(GLuint texture, FrameBuffer* fb)
{
    fb->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_high_luminance_region_extraction.SetUniform1i("texture0", 0);
    pipeline_high_luminance_region_extraction.SetUniform2f("pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

    pipeline_high_luminance_region_extraction.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    fs_pass_geom->Draw();

    glActiveTexture(GL_TEXTURE0);

    pipeline_high_luminance_region_extraction.Unbind();

    fb->Unbind();
}

void MyWindow::PassDownsampling2x2(GLuint texture, FrameBuffer* fb)
{
    fb->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_downsampling_2x2.SetUniform1i("texture0", 0);
    pipeline_downsampling_2x2.SetUniform2f("pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

    pipeline_downsampling_2x2.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    fs_pass_geom->Draw();

    glActiveTexture(GL_TEXTURE0);

    pipeline_downsampling_2x2.Unbind();

    fb->Unbind();
}

void MyWindow::PassDownsampling4x4(GLuint texture, FrameBuffer* fb)
{
    fb->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_downsampling_4x4.SetUniform1i("texture0", 0);
    pipeline_downsampling_4x4.SetUniform2f("pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

    pipeline_downsampling_4x4.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    fs_pass_geom->Draw();

    glActiveTexture(GL_TEXTURE0);

    pipeline_downsampling_4x4.Unbind();

    fb->Unbind();
}

void MyWindow::PassStreak(GLuint texture, FrameBuffer* fb, float dx, float dy, float attenuation, int pass)
{
    fb->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_streak.SetUniform1i("u_tex0", 0);
    pipeline_streak.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));
    pipeline_streak.SetUniform2f("u_direction", dx, dy);
    pipeline_streak.SetUniform2f("u_params", attenuation, static_cast<float>(pass));

    pipeline_streak.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    fs_pass_geom->Draw();

    glActiveTexture(GL_TEXTURE0);

    pipeline_streak.Unbind();

    fb->Unbind();
}

void MyWindow::PassApply(GLuint texture, FrameBuffer* fb)
{
    if(fb != nullptr)
        fb->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_apply.SetUniform1i("texture0", 0);
    pipeline_apply.SetUniform2f("pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

    pipeline_apply.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    fs_pass_geom->Draw();

    glActiveTexture(GL_TEXTURE0);

    pipeline_apply.Unbind();

    if(fb != nullptr)
        fb->Unbind();
}