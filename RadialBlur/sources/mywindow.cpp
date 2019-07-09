﻿#include <iomanip>
#include <sstream>
#include <map>
#include <GL/glew.h>
#include <hasenpfote/assert.h>
#include <hasenpfote/fp_conversion.h>
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
    {
        std::filesystem::path texpath;
        GLuint texture;

        texpath = "assets/textures/test_image_1.png";
        texture = System::GetConstInstance().GetTextureManager().GetTexture(texpath);
        selectable_textures.push_back(std::make_tuple(texture, texpath));

        texpath = "assets/textures/test_image_2.png";
        texture = System::GetConstInstance().GetTextureManager().GetTexture(texpath);
        selectable_textures.push_back(std::make_tuple(texture, texpath));

        texpath = "assets/textures/test_image_3.png";
        texture = System::GetConstInstance().GetTextureManager().GetTexture(texpath);
        selectable_textures.push_back(std::make_tuple(texture, texpath));

        selected_texture_index = 0;
    }

    fs_pass_geom = std::make_unique<FullscreenPassGeometry>();

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

    auto& man = System::GetConstInstance().GetShaderManager();
    pipeline_fullscreen_quad.Create();
    pipeline_fullscreen_quad.SetShaderProgram(man.GetShaderProgram("assets/shaders/fullscreen_quad.vs"));
    pipeline_fullscreen_quad.SetShaderProgram(man.GetShaderProgram("assets/shaders/fullscreen_quad.fs"));

    pipeline_high_luminance_region_extraction.Create();
    pipeline_high_luminance_region_extraction.SetShaderProgram(man.GetShaderProgram("assets/shaders/high_luminance_region_extraction.vs"));
    pipeline_high_luminance_region_extraction.SetShaderProgram(man.GetShaderProgram("assets/shaders/high_luminance_region_extraction.fs"));

    pipeline_simple_radial_blur.Create();
    pipeline_simple_radial_blur.SetShaderProgram(man.GetShaderProgram("assets/shaders/simple_radial_blur.vs"));
    pipeline_simple_radial_blur.SetShaderProgram(man.GetShaderProgram("assets/shaders/simple_radial_blur.fs"));

    pipeline_custom_radial_blur.Create();
    pipeline_custom_radial_blur.SetShaderProgram(man.GetShaderProgram("assets/shaders/custom_radial_blur.vs"));
    pipeline_custom_radial_blur.SetShaderProgram(man.GetShaderProgram("assets/shaders/custom_radial_blur.fs"));

    pipeline_apply.Create();
    pipeline_apply.SetShaderProgram(man.GetShaderProgram("assets/shaders/apply.vs"));
    pipeline_apply.SetShaderProgram(man.GetShaderProgram("assets/shaders/apply.fs"));

    origin_texcoord = hasenpfote::math::Vector2(0.5f, 0.5f);

    is_debug_enabled = false;
    is_filter_enabled = false;
    radial_blur_mode = 0;
    attn_coef = 0.95f;
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
    if(key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        is_filter_enabled = !is_filter_enabled;
    }
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        is_debug_enabled = !is_debug_enabled;
    }
    if(key == GLFW_KEY_F1 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        if(!selectable_textures.empty())
        {
            selected_texture_index += (mods == GLFW_MOD_SHIFT) ? -1 : 1;
            if(selected_texture_index < 0)
                selected_texture_index = selectable_textures.size() - 1;
            else if(selected_texture_index >= selectable_textures.size())
                selected_texture_index = 0;
        }
    }
    if(key == GLFW_KEY_F2 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
        {
            radial_blur_mode += (mods == GLFW_MOD_SHIFT) ? -1 : 1;
            if(radial_blur_mode < 0)
                radial_blur_mode = 2;
            else if(radial_blur_mode >= 3)
                radial_blur_mode = 0;
        }
    }
    if(key == GLFW_KEY_F3 && action == GLFW_PRESS)
    {
        if(is_filter_enabled)
        {
            attn_coef += (mods == GLFW_MOD_SHIFT) ? -0.01f : 0.01f;
            if(attn_coef < 0.0f)
                attn_coef = 0.0f;
            else if(attn_coef > 1.0f)
                attn_coef = 1.0f;
        }
    }
}

void MyWindow::OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    System::GetMutableInstance().GetCamera().OnMouseMove(xpos, ypos);
    mouse_pos = hasenpfote::math::Vector2(static_cast<float>(xpos), static_cast<float>(ypos));
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

    {
        auto& vp = camera.GetViewport();
        const auto width = vp.GetWidth();
        const auto height = vp.GetHeight();

        origin_texcoord = hasenpfote::math::Vector2(
            (mouse_pos.GetX() + 0.5f) / static_cast<float>(width),
            (static_cast<float>(height) - mouse_pos.GetY() + 0.5f) / static_cast<float>(height)
        );
    }
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

    // 1) Render scene to texture.
    auto texture = std::get<0>(selectable_textures[selected_texture_index]);
    scene_rt->Bind();
    DrawFullScreenQuad(texture);
    scene_rt->Unbind();

    // 2) Downsampling 4x4
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

    // 3) Radial Blur.
    if(is_filter_enabled)
    {
        if(radial_blur_mode == 0)
            PassSimpleRadialBlur(high_luminance_region_rt.get(), output_rt);
        else
            PassCustomRadialBlur(high_luminance_region_rt.get(), output_rt);
    }

    PassApply(output_rt); // Render HDR to LDR directly.

    // Display debug information.
    std::vector<std::string> text_lines;
    std::ostringstream oss;
    //std::streamsize ss = std::cout.precision();

    oss << std::fixed << std::setprecision(2);

    oss << "FPS:UPS=";
    oss << GetFPS() << ":" << GetUPS();
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Screen size:";
    oss << camera.GetViewport().GetWidth() << "x" << camera.GetViewport().GetHeight();
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Aov D=" << ConvertRadiansToDegrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Diagonal));
    oss << " H=" << ConvertRadiansToDegrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Horizontal));
    oss << " V=" << ConvertRadiansToDegrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Vertical));
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Focal length=" << camera.GetFocalLength() << " (35mm=" << camera.Get35mmEquivalentFocalLength() << ")";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Zoom=x" << camera.GetZoomMagnification();
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    GLboolean ms;
    glGetBooleanv(GL_MULTISAMPLE, &ms);
    oss << "MultiSample:" << ((ms == GL_TRUE) ? "On" : "Off") << "(Toggle MultiSample: m)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    const auto& texpath = std::get<1>(selectable_textures[selected_texture_index]);
    oss << "Texture:" << texpath << " ([Shift +] F1)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Radial Blur:" << (is_filter_enabled ? "On" : "Off") << "(Toggle Radial Blur: f)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Radial Blur Mode:" << ((radial_blur_mode == 0)? "Simple" : "Custom") << " ([Shift +] F2)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Attenuation Coefficient:" << attn_coef << " ([Shift +] F3)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Debug:" << (is_debug_enabled ? "On" : "Off") << "(Toggle Debug: p)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    DrawTextLines(text_lines);
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

    //
    auto ds_width = width / 4;
    auto ds_height = height / 4;

    System::GetMutableInstance().GetTextureManager().DeleteTexture("high_luminance_region_rt_color");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("high_luminance_region_rt_color", GL_RGBA16F, ds_width, ds_height);
    high_luminance_region_rt = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    System::GetMutableInstance().GetTextureManager().DeleteTexture("radial_blur_color_0");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("radial_blur_color_0", GL_RGBA16F, ds_width, ds_height);
    radial_blur_rts[0] = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    System::GetMutableInstance().GetTextureManager().DeleteTexture("radial_blur_color_1");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("radial_blur_color_1", GL_RGBA16F, ds_width, ds_height);
    radial_blur_rts[1] = std::make_unique<FrameBuffer>(color_texture, 0, 0);

    System::GetMutableInstance().GetTextureManager().DeleteTexture("radial_blur_color_2");
    color_texture = System::GetMutableInstance().GetTextureManager().CreateTexture("radial_blur_color_2", GL_RGBA16F, ds_width, ds_height);
    radial_blur_rts[2] = std::make_unique<FrameBuffer>(color_texture, 0, 0);
}

void MyWindow::DrawTextLines(std::vector<std::string> text_lines)
{
    if(text_lines.empty())
        return;

    using namespace hasenpfote::math;

    auto metrics = text->GetFont().GetFontMetrics();
    auto line_height = static_cast<float>(metrics.GetLineHeight());
    const float scale = 0.5f;
    const float fh = line_height * scale;

    static const Vector4 color(1.0f, 1.0f, 1.0f, 1.0f);
    text->SetColor(static_cast<const GLfloat*>(color));

    text->BeginRendering();
    int line_no = 1;
    for(const auto& text_line : text_lines)
    {
        text->DrawString(text_line, 0.0f, fh * static_cast<float>(line_no), scale);
        line_no++;
    }

    text->EndRendering();
}

void MyWindow::DrawFullScreenQuad(GLuint texture)
{
    pipeline_fullscreen_quad.SetUniform1i("u_tex0", 0);

    pipeline_fullscreen_quad.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, linear_sampler);

    fs_pass_geom->Draw();

    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

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
        pipeline_high_luminance_region_extraction.SetUniform1f("u_exposure", 1.0f);
        pipeline_high_luminance_region_extraction.SetUniform1f("u_threshold", 0.35f);
        pipeline_high_luminance_region_extraction.SetUniform1f("u_soft_threshold", 0.5f);

        pipeline_high_luminance_region_extraction.Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_pass_geom->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_high_luminance_region_extraction.Unbind();
    }
    output->Unbind();
}

void MyWindow::PassSimpleRadialBlur(FrameBuffer* input, FrameBuffer* output)
{
    float ox = origin_texcoord.GetX();
    float oy = origin_texcoord.GetY();

    PassSimpleRadialBlur(input, radial_blur_rts[0].get(), ox, oy, attn_coef);

    //
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        PassApply(radial_blur_rts[0].get(), output);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void MyWindow::PassSimpleRadialBlur(FrameBuffer* input, FrameBuffer* output, float x, float y, float attenuation)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        pipeline_simple_radial_blur.SetUniform1i("u_tex0", 0);
        pipeline_simple_radial_blur.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));
        pipeline_simple_radial_blur.SetUniform2f("u_origin", x, y);
        pipeline_simple_radial_blur.SetUniform1f("u_attenuation", attenuation);

        pipeline_simple_radial_blur.Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_pass_geom->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_simple_radial_blur.Unbind();

    }
    output->Unbind();
}

void MyWindow::PassCustomRadialBlur(FrameBuffer* input, FrameBuffer* output)
{
    const int num_of_passes = 5;

    float ox = origin_texcoord.GetX();
    float oy = origin_texcoord.GetY();

    auto last_rt = input;

    for(std::remove_const<decltype(num_of_passes)>::type pass = 0; pass < num_of_passes; pass++)
    {
        decltype(last_rt) src_rt = last_rt;
        decltype(last_rt) dst_rt = ((pass % 2) == 0) ? radial_blur_rts[1].get() : radial_blur_rts[2].get();
        PassCustomRadialBlur(src_rt, dst_rt, ox, oy, attn_coef, pass, num_of_passes);
        last_rt = dst_rt;
    }
    PassApply(last_rt, radial_blur_rts[0].get());

    //
    {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        PassApply(radial_blur_rts[0].get(), output);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void MyWindow::PassCustomRadialBlur(FrameBuffer* input, FrameBuffer* output, float x, float y, float attenuation, int pass, int num_of_passes)
{
    output->Bind();
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        pipeline_custom_radial_blur.SetUniform1i("u_tex0", 0);
        pipeline_custom_radial_blur.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));
        pipeline_custom_radial_blur.SetUniform2f("u_origin", x, y);
        pipeline_custom_radial_blur.SetUniform3f("u_params", attenuation, static_cast<float>(pass), static_cast<float>(num_of_passes));

        pipeline_custom_radial_blur.Bind();
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
            glBindSampler(0, linear_sampler);

            fs_pass_geom->Draw();

            glBindSampler(0, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        pipeline_custom_radial_blur.Unbind();

    }
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
        glBindSampler(0, linear_sampler);

        fs_pass_geom->Draw();

        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_apply.Unbind();

    if(output != nullptr)
        output->Unbind();
}