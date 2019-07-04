#include <iomanip>
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
        auto& man = System::GetMutableInstance().GetTextureManager();
        man.LoadTextures(directory);
    }
    // generate font.
    {
        std::filesystem::path fontpath = "../Common/assets/fonts/test.fnt";
        auto font = std::make_shared<text::Font>(fontpath);
        text = std::make_unique<SDFText>(font, std::make_shared<SDFTextRenderer>());
        text->SetSmoothness(1.0f);
    }
    //
    auto texture = System::GetConstInstance().GetTextureManager().GetTexture("assets/textures/street_lamp_1k.exr");
    textures.push_back(texture);
    texture = System::GetConstInstance().GetTextureManager().GetTexture("assets/textures/satara_night_no_lamps_1k.exr");
    textures.push_back(texture);
    selected_texture_index = 0;

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

    pipeline_high_luminance_region_extraction.Create();
    pipeline_high_luminance_region_extraction.SetShaderProgram(man.GetShaderProgram("assets/shaders/high_luminance_region_extraction.vs"));
    pipeline_high_luminance_region_extraction.SetShaderProgram(man.GetShaderProgram("assets/shaders/high_luminance_region_extraction.fs"));

    pipeline_downsampling_2x2.Create();
    pipeline_downsampling_2x2.SetShaderProgram(man.GetShaderProgram("assets/shaders/downsampling.vs"));
    pipeline_downsampling_2x2.SetShaderProgram(man.GetShaderProgram("assets/shaders/downsampling_2x2.fs"));

    pipeline_downsampling_4x4.Create();
    pipeline_downsampling_4x4.SetShaderProgram(man.GetShaderProgram("assets/shaders/downsampling.vs"));
    pipeline_downsampling_4x4.SetShaderProgram(man.GetShaderProgram("assets/shaders/downsampling_4x4.fs"));

    pipeline_kawase_blur.Create();
    pipeline_kawase_blur.SetShaderProgram(man.GetShaderProgram("assets/shaders/kawase_blur.vs"));
    pipeline_kawase_blur.SetShaderProgram(man.GetShaderProgram("assets/shaders/kawase_blur.fs"));

    pipeline_streak.Create();
    pipeline_streak.SetShaderProgram(man.GetShaderProgram("assets/shaders/streak.vs"));
    pipeline_streak.SetShaderProgram(man.GetShaderProgram("assets/shaders/streak.fs"));

    pipeline_tonemapping.Create();
    pipeline_tonemapping.SetShaderProgram(man.GetShaderProgram("assets/shaders/tonemapping.vs"));
    pipeline_tonemapping.SetShaderProgram(man.GetShaderProgram("assets/shaders/tonemapping.fs"));

    pipeline_apply.Create();
    pipeline_apply.SetShaderProgram(man.GetShaderProgram("assets/shaders/apply.vs"));
    pipeline_apply.SetShaderProgram(man.GetShaderProgram("assets/shaders/apply.fs"));

    exposure = 2.0f;
    is_bloom_enabled = false;
    is_streak_enabled = false;
    is_debug_enabled = false;
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
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        is_debug_enabled = !is_debug_enabled;
    }
    if(key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        is_bloom_enabled = !is_bloom_enabled;
    }
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        is_streak_enabled = !is_streak_enabled;
    }
    if(key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        exposure += 0.1f;
    }
    if(key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        exposure -= 0.1f;
        if(exposure < 0.0f)
            exposure = 0.0f;
    }
    if(key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        if(!textures.empty())
        {
            selected_texture_index--;
            if(selected_texture_index < 0)
                selected_texture_index = textures.size() - 1;
        }
    }
    if(key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        if(!textures.empty())
        {
            selected_texture_index++;
            if(selected_texture_index >= textures.size())
                selected_texture_index = 0;
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
    DrawFullScreenQuad(textures[selected_texture_index]);
    scene_rt->Unbind();

    // 2) 高輝度領域の抽出(兼ダウンサンプリング)
    PassHighLuminanceRegionExtraction(scene_rt.get(), high_luminance_region_rt.get());

    FrameBuffer* output_rt;
    if(is_debug_enabled)
    {
        GLfloat clear_color[] = { 0.0f, 0.0, 0.0f, 1.0f };
        glClearTexImage(debug_rt->GetColorTexture(), 0, GL_RGBA, GL_FLOAT, &clear_color);
        output_rt = debug_rt.get();
    }
    else
    {
        output_rt = scene_rt.get();
    }

    // 3) Bloom 
    if(is_bloom_enabled)
        PassBloom(high_luminance_region_rt.get(), output_rt);

    // 4) Streak
    if(is_streak_enabled)
        PassStreak(high_luminance_region_rt.get(), output_rt);

    PassTonemapping(output_rt);

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

    oss << "Exposure=" << exposure << " (Up / Down)";
    text->DrawString(oss.str(), 0.0f, fh * 7.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Debug:" << (is_debug_enabled ? "On" : "Off") << "(Debug: p)";
    text->DrawString(oss.str(), 0.0f, fh * 8.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Bloom:" << (is_bloom_enabled ? "On" : "Off") << "(Bloom: b)";
    text->DrawString(oss.str(), 0.0f, fh * 9.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Streak:" << (is_streak_enabled ? "On" : "Off") << "(Streak: l)";
    text->DrawString(oss.str(), 0.0f, fh * 10.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    text->EndRendering();
}

void MyWindow::RecreateResources(int width, int height)
{
    GLuint color_texture;

    // for scene.
    System::GetMutableInstance().GetTextureManager().DeleteTexture("scene_rt_color");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("scene_rt_color", GL_RGBA16F, width, height);
    scene_rt = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    // for debug.
    System::GetMutableInstance().GetTextureManager().DeleteTexture("debug_rt_color");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("debug_rt_color", GL_RGBA16F, width, height);
    debug_rt = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    // for high luminance region.
    auto ds_width = width / 4;
    auto ds_height = height / 4;

    System::GetMutableInstance().GetTextureManager().DeleteTexture("high_luminance_region_rt_color");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("high_luminance_region_rt_color", GL_RGBA16F, ds_width, ds_height);
    high_luminance_region_rt = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    // for bloom.
    bloom_rts.clear();
    std::stringstream ss;
    for(auto i = 0; i < 5; i++)
    {
        ss << "bloom_rts_color_" << i << "_" << 0;
        System::GetMutableInstance().GetTextureManager().DeleteTexture(ss.str());
        color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture(ss.str(), GL_RGBA16F, ds_width, ds_height);
        auto bloom_rt_0 = std::make_unique<FrameBuffer>(color_texture, 0, 0);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        ss << "bloom_rts_color_" << i << "_" << 1;
        System::GetMutableInstance().GetTextureManager().DeleteTexture(ss.str());
        color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture(ss.str(), GL_RGBA16F, ds_width, ds_height);
        auto bloom_rt_1 = std::make_unique<FrameBuffer>(color_texture, 0, 0);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        ss << "bloom_rts_color_" << i << "_" << 2;
        System::GetMutableInstance().GetTextureManager().DeleteTexture(ss.str());
        color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture(ss.str(), GL_RGBA16F, ds_width, ds_height);
        auto bloom_rt_2 = std::make_unique<FrameBuffer>(color_texture, 0, 0);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        bloom_rts.push_back({
            std::move(bloom_rt_0),
            std::move(bloom_rt_1),
            std::move(bloom_rt_2)
            });

        ds_width /= 2;
        ds_height /= 2;
    }

    // for streak.
    ds_width = width / 4;
    ds_height = height / 4;
    {
        ss << "streak_rt_color_" << 0;
        System::GetMutableInstance().GetTextureManager().DeleteTexture(ss.str());
        color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture(ss.str(), GL_RGBA16F, ds_width, ds_height);
        auto streak_rt_0 = std::make_unique<FrameBuffer>(color_texture, 0, 0);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        ss << "streak_rt_color_" << 1;
        System::GetMutableInstance().GetTextureManager().DeleteTexture(ss.str());
        color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture(ss.str(), GL_RGBA16F, ds_width, ds_height);
        auto streak_rt_1 = std::make_unique<FrameBuffer>(color_texture, 0, 0);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        ss << "streak_rt_color_" << 2;
        System::GetMutableInstance().GetTextureManager().DeleteTexture(ss.str());
        color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture(ss.str(), GL_RGBA16F, ds_width, ds_height);
        auto streak_rt_2 = std::make_unique<FrameBuffer>(color_texture, 0, 0);
        ss.str("");
        ss.clear(std::stringstream::goodbit);

        streak_rts = { std::move(streak_rt_0), std::move(streak_rt_1), std::move(streak_rt_2) };
    }
}

void MyWindow::DrawFullScreenQuad(GLuint texture)
{
    pipeline_fullscreen_quad.SetUniform1i("u_tex0", 0);

    pipeline_fullscreen_quad.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);

    fs_pass_geom->Draw();

    glActiveTexture(GL_TEXTURE0);

    pipeline_fullscreen_quad.Unbind();
}

void MyWindow::PassHighLuminanceRegionExtraction(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        pipeline_high_luminance_region_extraction.SetUniform1i("u_tex0", 0);
        pipeline_high_luminance_region_extraction.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));
        pipeline_high_luminance_region_extraction.SetUniform1f("u_exposure", exposure);
        pipeline_high_luminance_region_extraction.SetUniform1f("u_threshold", 1.0f);
        pipeline_high_luminance_region_extraction.SetUniform1f("u_soft_threshold", 0.5f);

        pipeline_high_luminance_region_extraction.Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_pass_geom->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_high_luminance_region_extraction.Unbind();
    }
    output->Unbind();
}

void MyWindow::PassDownsampling2x2(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        pipeline_downsampling_2x2.SetUniform1i("u_tex0", 0);
        pipeline_downsampling_2x2.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

        pipeline_downsampling_2x2.Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_pass_geom->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_downsampling_2x2.Unbind();
    }
    output->Unbind();
}

void MyWindow::PassDownsampling4x4(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        pipeline_downsampling_4x4.SetUniform1i("u_tex0", 0);
        pipeline_downsampling_4x4.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

        pipeline_downsampling_4x4.Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_pass_geom->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_downsampling_4x4.Unbind();
    }
    output->Unbind();
}

void MyWindow::PassKawaseBlur(FrameBuffer* input, FrameBuffer* output, int iteration)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        pipeline_kawase_blur.SetUniform1i("u_tex0", 0);
        pipeline_kawase_blur.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));
        pipeline_kawase_blur.SetUniform1f("u_iteration", static_cast<float>(iteration));

        pipeline_kawase_blur.Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_pass_geom->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_kawase_blur.Unbind();
    }
    output->Unbind();
}

void MyWindow::PassBloom(FrameBuffer* input, FrameBuffer* output)
{
    // 1) ダウンサンプリング
    {
        auto last_rt = bloom_rts[0][0].get();
        PassApply(input, last_rt);

        auto size = bloom_rts.size();
        for(auto i = 1; i < size; i++)
        {
            auto src_rt = last_rt;
            auto dst_rt = bloom_rts[i][0].get();
            PassDownsampling2x2(src_rt, dst_rt);
            last_rt = dst_rt;
        }
    }
    // 2) 各レベルにピンポンブラー
    {
        std::vector<int> kernel = { 0, 0 };
        const auto num_of_passes = kernel.size();

        for(auto& bloom_rt : bloom_rts)
        {
            auto last_rt = bloom_rt[0].get();
            for(auto i = 0; i < num_of_passes; i++)
            {
                FrameBuffer* src_rt = last_rt;
                FrameBuffer* dst_rt = ((i % 2) == 0) ? bloom_rt[1].get() : bloom_rt[2].get();
                PassKawaseBlur(src_rt, dst_rt, kernel[i]);
                last_rt = dst_rt;
            }
            PassApply(last_rt, bloom_rt[0].get());
        }
    }
    // 3) 各フィルタを合成
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        auto size = bloom_rts.size();

        for(auto i = size - 1; i > i - 1; i--)
        {
            PassApply(bloom_rts[i][0].get(), bloom_rts[i-1][0].get());
        }
        PassApply(bloom_rts[0][0].get(), output);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void MyWindow::PassStreak(FrameBuffer* input, FrameBuffer* output)
{
    // 1) 方向毎にピンポンブラー
    {
        const auto num_of_streaks = 4;
        const auto num_of_passes = 3;
        const auto additional_angle = 360.0f / static_cast<float>(num_of_streaks);

        GLfloat clear_color[] = { 0.0f, 0.0, 0.0f, 1.0f };
        glClearTexImage(streak_rts[0]->GetColorTexture(), 0, GL_RGBA, GL_FLOAT, &clear_color);

        float angle = 45.0f;
        for(auto i = 0; i < num_of_streaks; i++)
        {
            auto theta = hasenpfote::math::ConvertDegreesToRadians(angle);
            auto dx = std::cos(theta);
            auto dy = std::sin(theta);

            auto last_rt = input;
            for(auto j = 0; j < num_of_passes; j++)
            {
                FrameBuffer* src_rt = last_rt;
                FrameBuffer* dst_rt = ((j % 2) == 0) ? streak_rts[1].get() : streak_rts[2].get();
                PassStreak(src_rt, dst_rt, dx, dy, 0.90f, j);
                last_rt = dst_rt;
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            PassApply(last_rt, streak_rts[0].get());
            glDisable(GL_BLEND);

            angle += additional_angle;
        }
    }

    // 2) 各フィルタを合成
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        PassApply(streak_rts[0].get(), output);
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

        pipeline_streak.SetUniform1i("u_tex0", 0);
        pipeline_streak.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));
        pipeline_streak.SetUniform2f("u_direction", dx, dy);
        pipeline_streak.SetUniform2f("u_params", attenuation, static_cast<float>(pass));

        pipeline_streak.Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, sampler);

            fs_pass_geom->Draw();

            glActiveTexture(GL_TEXTURE0);
        }
        pipeline_streak.Unbind();
    }
    output->Unbind();
}

void MyWindow::PassTonemapping(FrameBuffer* input, FrameBuffer* output)
{
    if(output != nullptr)
        output->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_tonemapping.SetUniform1i("u_tex0", 0);
    pipeline_tonemapping.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));
    pipeline_tonemapping.SetUniform1f("u_exposure", exposure);

    pipeline_tonemapping.Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, sampler);

        fs_pass_geom->Draw();

        glActiveTexture(GL_TEXTURE0);
    }
    pipeline_tonemapping.Unbind();

    if(output != nullptr)
        output->Unbind();
}

void MyWindow::PassApply(FrameBuffer* input, FrameBuffer* output)
{
    if(output != nullptr)
        output->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_apply.SetUniform1i("u_tex0", 0);
    pipeline_apply.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

    pipeline_apply.Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, sampler);

        fs_pass_geom->Draw();

        glActiveTexture(GL_TEXTURE0);
    }
    pipeline_apply.Unbind();

    if(output != nullptr)
        output->Unbind();
}