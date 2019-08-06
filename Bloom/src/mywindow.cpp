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
#include "../../common/system.h"
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
    {"test",                {0, 1, 2}},
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
        std::filesystem::path dirpath("assets/shaders");
        auto& rm = System::GetMutableInstance().GetResourceManager();
        rm.AddResourcesFromDirectory<common::ShaderProgram>(dirpath, false);
    }

    // load texture.
    {
        std::filesystem::path dirpath("assets/textures");
        auto& rm = System::GetMutableInstance().GetResourceManager();
        rm.AddResourcesFromDirectory<common::Texture>(dirpath, false);
    }
    // generate font.
    {
        std::filesystem::path fontpath = "../common/assets/fonts/test.fnt";
        auto font = std::make_shared<text::Font>(fontpath);
        text = std::make_unique<SDFText>(font, std::make_shared<SDFTextRenderer>());
        text->SetSmoothness(1.0f);
    }
    //
    auto& rm = System::GetConstInstance().GetResourceManager();

    texture = rm.GetResource<common::Texture>("assets/textures/testimg_1920x1080.png")->GetTexture();

    fs_pass_geom = std::make_unique<FullscreenPassGeometry>();

    glGenSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_LOD_BIAS, 0.0f);

    RecreateResources(width, height);

    pipeline_fullscreen_quad.Create();
    pipeline_fullscreen_quad.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/fullscreen_quad.vs"));
    pipeline_fullscreen_quad.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/fullscreen_quad.fs"));

    pipeline_apply.Create();
    pipeline_apply.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/apply.vs"));
    pipeline_apply.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/apply.fs"));

    pipeline_downsampling_2x2.Create();
    pipeline_downsampling_2x2.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/downsampling.vs"));
    pipeline_downsampling_2x2.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/downsampling_2x2.fs"));

    pipeline_downsampling_4x4.Create();
    pipeline_downsampling_4x4.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/downsampling.vs"));
    pipeline_downsampling_4x4.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/downsampling_4x4.fs"));

    pipeline_kawase_blur.Create();
    pipeline_kawase_blur.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/kawase_blur.vs"));
    pipeline_kawase_blur.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/kawase_blur.fs"));

    pipeline_high_luminance_region_extraction.Create();
    pipeline_high_luminance_region_extraction.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/high_luminance_region_extraction.vs"));
    pipeline_high_luminance_region_extraction.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/high_luminance_region_extraction.fs"));

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
    if(key == GLFW_KEY_7 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
            shader_kernel_name = "test";
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
    PassHighLuminanceRegionExtraction(scene_rt.get(), high_luminance_region_rt.get());

    FrameBuffer* output_rt = scene_rt.get();

    // 3) Bloom を適用
    if(is_filter_enabled)
        PassBloom(high_luminance_region_rt.get(), output_rt);

    glEnable(GL_FRAMEBUFFER_SRGB);
    PassApply(output_rt);
    glDisable(GL_FRAMEBUFFER_SRGB);

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

    oss << "Kawase blur:" << ((is_filter_enabled == GL_TRUE) ? "On" : "Off") << "(Toggle Kawase blur: b)" << " " << shader_kernel_name;
    text->DrawString(oss.str(), 0.0f, fh * 7.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    text->EndRendering();
}

void MyWindow::RecreateResources(int width, int height)
{
    auto recreate_fb = [](const std::string& name, GLsizei levels, GLenum internal_format, GLsizei width, GLsizei height)
    {
        auto& rm = System::GetMutableInstance().GetResourceManager();

        rm.RemoveResource<common::Texture>(name);

        if(levels == 0)
            levels = common::Texture::CalcNumOfMipmapLevels(width, height);

        auto p = std::make_unique<common::Texture>(levels, internal_format, width, height);
        auto texture = p->GetTexture();

        rm.AddResource<common::Texture>(name, std::move(p));

        return std::make_unique<FrameBuffer>(texture, 0, 0);
    };

    // for scene.
    {
        const auto name = std::string("scene_rt_color");
        scene_rt = recreate_fb(name, 1, GL_RGBA16F, width, height);
    }
    // for high luminance region.
    {
        auto ds_width = width / 2;
        auto ds_height = height / 2;
        const auto name = std::string("luminance_rt_color");
        high_luminance_region_rt = recreate_fb(name, 1, GL_RGBA16F, ds_width, ds_height);
    }
    {
        std::stringstream ss;

        auto ds_width = width / 4;
        auto ds_height = height / 4;

        downsampled_rts.clear();

        for(auto i = 0; i < 5; i++)
        {
            ds_width /= 2;
            ds_height /= 2;

            ss << "downsampled_rt_color_" << i << "_" << 0;
            auto downsampled_rt_0 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
            ss.str("");
            ss.clear(std::stringstream::goodbit);

            ss << "downsampled_rt_color_" << i << "_" << 1;
            auto downsampled_rt_1 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
            ss.str("");
            ss.clear(std::stringstream::goodbit);

            ss << "downsampled_rt_color_" << i << "_" << 2;
            auto downsampled_rt_2 = recreate_fb(ss.str(), 1, GL_RGBA16F, ds_width, ds_height);
            ss.str("");
            ss.clear(std::stringstream::goodbit);

            downsampled_rts.push_back({
                std::move(downsampled_rt_0),
                std::move(downsampled_rt_1),
                std::move(downsampled_rt_2)
                });
        }
    }
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

void MyWindow::PassHighLuminanceRegionExtraction(FrameBuffer* input, FrameBuffer* output)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        pipeline_high_luminance_region_extraction.SetUniform1i("texture0", 0);
        pipeline_high_luminance_region_extraction.SetUniform2f("pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

        pipeline_high_luminance_region_extraction.Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, sampler);

        fs_pass_geom->Draw();

        glActiveTexture(GL_TEXTURE0);

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

        pipeline_downsampling_2x2.SetUniform1i("texture0", 0);
        pipeline_downsampling_2x2.SetUniform2f("pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

        pipeline_downsampling_2x2.Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, sampler);

        fs_pass_geom->Draw();

        glActiveTexture(GL_TEXTURE0);

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

        pipeline_downsampling_4x4.SetUniform1i("texture0", 0);
        pipeline_downsampling_4x4.SetUniform2f("pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

        pipeline_downsampling_4x4.Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, sampler);

        fs_pass_geom->Draw();

        glActiveTexture(GL_TEXTURE0);

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

        pipeline_kawase_blur.SetUniform1i("texture0", 0);
        pipeline_kawase_blur.SetUniform3f("params", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]), static_cast<float>(iteration));

        pipeline_kawase_blur.Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, sampler);

        fs_pass_geom->Draw();

        glActiveTexture(GL_TEXTURE0);

        pipeline_kawase_blur.Unbind();
    }
    output->Unbind();
}

void MyWindow::PassBloom(FrameBuffer* input, FrameBuffer* output)
{
    // 1) ダウンサンプリング
    {
        auto last_rt = input;
        auto size = downsampled_rts.size();
        for(decltype(size) i = 0; i < size; i++)
        {
            auto src_rt = last_rt;
            auto dst_rt = downsampled_rts[i][0].get();
            PassDownsampling2x2(src_rt, dst_rt);
            last_rt = dst_rt;
        }
    }
    // 2) 各レベルにピンポンブラー
    {
        auto it = shader_kernel.find(shader_kernel_name);
        assert(it != shader_kernel.cend());
        auto kernel = it->second;
        const auto num_of_passes = kernel.size();

        for(auto& downsampled_rt : downsampled_rts)
        {
            auto last_rt = downsampled_rt[0].get();
            for(std::remove_const<decltype(num_of_passes)>::type i = 0; i < num_of_passes; i++)
            {
                FrameBuffer* src_rt = last_rt;
                FrameBuffer* dst_rt = ((i % 2) == 0) ? downsampled_rt[1].get() : downsampled_rt[2].get();
                PassKawaseBlur(src_rt, dst_rt, kernel[i]);
                last_rt = dst_rt;
            }
            PassApply(last_rt, downsampled_rt[0].get());
        }
    }
    // 3) 各フィルタを合成
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        auto size = downsampled_rts.size();

        for(decltype(size) i = size - 1; i > i - 1; i--)
        {
            PassApply(downsampled_rts[i][0].get(), downsampled_rts[i - 1][0].get());
        }
        PassApply(downsampled_rts[0][0].get(), output);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void MyWindow::PassApply(FrameBuffer* input, FrameBuffer* output)
{
    if(output != nullptr)
        output->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_apply.SetUniform1i("texture0", 0);
    pipeline_apply.SetUniform2f("pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

    pipeline_apply.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
    glBindSampler(0, sampler);

    fs_pass_geom->Draw();

    glActiveTexture(GL_TEXTURE0);

    pipeline_apply.Unbind();

    if(output != nullptr)
        output->Unbind();
}