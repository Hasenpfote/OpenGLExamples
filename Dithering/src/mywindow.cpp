#include <iomanip>
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
#include "../../common/logger.h"
#include "bayer.h"
#include "mywindow.h"

const std::vector<std::tuple<std::string, std::size_t>> dither_settings =
{
    std::make_tuple<std::string, std::size_t>("bayer_2x2", 1),
    std::make_tuple<std::string, std::size_t>("bayer_4x4", 2),
    std::make_tuple<std::string, std::size_t>("bayer_8x8", 3),
    std::make_tuple<std::string, std::size_t>("bayer_16x16", 4),
    std::make_tuple<std::string, std::size_t>("bayer_32x32", 5),
    std::make_tuple<std::string, std::size_t>("bayer_64x64", 6),
    std::make_tuple<std::string, std::size_t>("bayer_128x128", 7),
    std::make_tuple<std::string, std::size_t>("bayer_256x256", 8),
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
        rm.AddResourcesFromDirectory<ShaderProgram>(dirpath, false);
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
    //
    {
        auto& rm = System::GetConstInstance().GetResourceManager();

        std::filesystem::path texpath;
        GLuint texture;

        texpath = "assets/textures/testimg_1920x1080.png";
        texture = rm.GetResource<Texture>(texpath.string())->GetTexture();
        selectable_textures.push_back(std::make_tuple(texture, texpath));

        selected_texture_index = 0;
    }
    // for dithering.
    {
        auto& rm = System::GetMutableInstance().GetResourceManager();

        for(const auto& dither_setting : dither_settings)
        {
            const auto& name = std::get<0>(dither_setting);
            auto level = std::get<1>(dither_setting);

            auto bayer_matrix = make_bayer_matrix(level);
            auto size = bayer_matrix.size();
            auto dim = static_cast<std::size_t>(std::sqrt(size));

            std::vector<std::uint8_t> temp(size);
            for(decltype(size) i = 0; i < size; i++)
            {
                temp[i] = static_cast<std::uint8_t>(bayer_matrix[i] * 255.0f);
            }

            auto p = std::make_unique<Texture>(GL_R8, dim, dim);
            auto texture = p->GetTexture();
            rm.AddResource<Texture>(name, std::move(p));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dim, dim, GL_RED, GL_UNSIGNED_BYTE, temp.data());
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        selected_dither_setting_index = 0;
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

    auto& rm = System::GetConstInstance().GetResourceManager();

    pipeline_fullscreen_quad.Create();
    pipeline_fullscreen_quad.SetShaderProgram(rm.GetResource<ShaderProgram>("assets/shaders/fullscreen_quad.vs"));
    pipeline_fullscreen_quad.SetShaderProgram(rm.GetResource<ShaderProgram>("assets/shaders/fullscreen_quad.fs"));

    pipeline_dithering.Create();
    pipeline_dithering.SetShaderProgram(rm.GetResource<ShaderProgram>("assets/shaders/dithering.vs"));
    pipeline_dithering.SetShaderProgram(rm.GetResource<ShaderProgram>("assets/shaders/dithering.fs"));

    pipeline_apply.Create();
    pipeline_apply.SetShaderProgram(rm.GetResource<ShaderProgram>("assets/shaders/apply.vs"));
    pipeline_apply.SetShaderProgram(rm.GetResource<ShaderProgram>("assets/shaders/apply.fs"));

    is_dithering_enabled = false;
    dithering_mode = 0;
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
    if(key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        is_dithering_enabled = !is_dithering_enabled;
    }
    if(key == GLFW_KEY_F1 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        if(!selectable_textures.empty())
        {
            selected_texture_index += (mods == GLFW_MOD_SHIFT) ? -1 : 1;
            if(selected_texture_index < 0)
                selected_texture_index = selectable_textures.size() - 1;
            else
            if(selected_texture_index >= selectable_textures.size())
                selected_texture_index = 0;
        }
    }
    if(key == GLFW_KEY_F2 && action == GLFW_PRESS)
    {
        if(is_dithering_enabled && !dither_settings.empty())
        {
            selected_dither_setting_index += (mods == GLFW_MOD_SHIFT) ? -1 : 1;
            if(selected_dither_setting_index < 0)
                selected_dither_setting_index = dither_settings.size() - 1;
            else
            if(selected_dither_setting_index >= dither_settings.size())
                selected_dither_setting_index = 0;
        }
    }
    if(key == GLFW_KEY_F3 && action == GLFW_PRESS)
    {
        if(is_dithering_enabled)
        {
            dithering_mode += (mods == GLFW_MOD_SHIFT) ? -1 : 1;
            if(dithering_mode < 0)
                dithering_mode = 1;
            else
            if(dithering_mode >= 2)
                dithering_mode = 0;
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

    // 1) Render scene to texture.
    auto texture = std::get<0>(selectable_textures[selected_texture_index]);
    scene_rt->Bind();
    DrawFullScreenQuad(texture);
    scene_rt->Unbind();

    // 2) Dithering
    glEnable(GL_FRAMEBUFFER_SRGB);

    if(is_dithering_enabled)
        PassDithering(scene_rt.get());
    else
        PassApply(scene_rt.get());

    glDisable(GL_FRAMEBUFFER_SRGB);

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

    oss << "Dithering:" << ((is_dithering_enabled) ? "On" : "Off") << "(Toggle Dithering: t)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    const auto& dither_setting = dither_settings[selected_dither_setting_index];
    oss << "Dithering Setting:" << selected_dither_setting_index << " - " << std::get<0>(dither_setting) << " ([Shift +] F2)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Dithering Mode:" << dithering_mode << " ([Shift +] F3)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    DrawTextLines(text_lines);
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

    // for scene.
    {
        const auto name = std::string("scene_rt_color");
        scene_rt = recreate_fb(name, 1, GL_RGBA16F, width, height);
    }
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

void MyWindow::PassDithering(FrameBuffer* input, FrameBuffer* output)
{
    if(output != nullptr)
        output->Bind();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    //
    //dither_setting_index
    const auto& dither_setting = dither_settings[selected_dither_setting_index];
    const auto& name = std::get<0>(dither_setting);
    const auto level = std::get<1>(dither_setting);
    const auto dim = static_cast<std::size_t>(std::pow(2.0f, level));

    auto& rm = System::GetConstInstance().GetResourceManager();
    const auto dither_texture = rm.GetResource<Texture>(name)->GetTexture();

    pipeline_dithering.SetUniform1i("u_tex0", 0);
    pipeline_dithering.SetUniform1i("u_tex1", 1);
    pipeline_dithering.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));
    pipeline_dithering.SetUniform1f("u_dimension", static_cast<float>(dim));
    pipeline_dithering.SetUniform1i("u_mode", dithering_mode);

    pipeline_dithering.Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dither_texture);
        glBindSampler(0, nearest_sampler);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(1, linear_sampler);

        fs_pass_geom->Draw();

        glBindSampler(1, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_dithering.Unbind();

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
        glBindSampler(0, linear_sampler);

        fs_pass_geom->Draw();

        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_apply.Unbind();

    if(output != nullptr)
        output->Unbind();
}