#include <iomanip>
#include <sstream>
#include <GL/glew.h>
#include <hasenpfote/assert.h>
#include <hasenpfote/fp_conversion.h>
#include <hasenpfote//math/utils.h>
#include <hasenpfote/math/vector3.h>
#include <hasenpfote/math/vector4.h>
#include <hasenpfote/math/cmatrix4.h>
#include <hasenpfote/math/axis_angle.h>
#include "../../common/system.h"
#include "../../common/logger.h"
#include "debug_utils.h"
#include "mywindow.h"

const std::vector<std::string> conversion_settings =
{
    "Linear to Linear",
    "Linear to Linear (with GL_FRAMEBUFFER_SRGB)",
    "Linear to sRGB",
    "Linear to sRGB (with GL_FRAMEBUFFER_SRGB)",
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
    dump_default_framebuffer_info();

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

    auto& rm = System::GetConstInstance().GetResourceManager();
    //
    {
        std::filesystem::path texpath;
        GLuint texture;

        texpath = "assets/textures/testimg_1920x1080.png";
        texture = rm.GetResource<common::Texture>(texpath.string())->GetTexture();
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

    pipeline_fullscreen_quad.Create();
    pipeline_fullscreen_quad.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/fullscreen_quad.vs"));
    pipeline_fullscreen_quad.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/fullscreen_quad.fs"));

    pipeline_linear_to_linear.Create();
    pipeline_linear_to_linear.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/linear_to_linear.vs"));
    pipeline_linear_to_linear.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/linear_to_linear.fs"));

    pipeline_linear_to_srgb.Create();
    pipeline_linear_to_srgb.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/linear_to_srgb.vs"));
    pipeline_linear_to_srgb.SetShaderProgram(rm.GetResource<common::ShaderProgram>("assets/shaders/linear_to_srgb.fs"));

    conversion_mode = 0;
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
        conversion_mode += (mods == GLFW_MOD_SHIFT) ? -1 : 1;
        if(conversion_mode < 0)
            conversion_mode = conversion_settings.size() - 1;
        else
        if(conversion_mode >= conversion_settings.size())
            conversion_mode = 0;
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

    assert(!glIsEnabled(GL_FRAMEBUFFER_SRGB));

    // 1) Render scene to texture.
    auto texture = std::get<0>(selectable_textures[selected_texture_index]);
    scene_rt->Bind();
    DrawFullScreenQuad(texture);
    scene_rt->Unbind();

    // 2) Render to default framebuffer.
    if(conversion_mode == 0)
    {
        PassLinearToLinear(scene_rt.get());
    }
    else if(conversion_mode == 1)
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
        PassLinearToLinear(scene_rt.get());
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
    else if(conversion_mode == 2)
    {
        PassLinearToSRGB(scene_rt.get());
    }
    else if(conversion_mode == 3)
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
        PassLinearToSRGB(scene_rt.get());
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
    else
    {
        assert(false);
    }

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

    oss << "Conversion Setting:" << conversion_mode << " - " << conversion_settings[conversion_mode] << " ([Shift +] F2)";
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

void MyWindow::PassLinearToLinear(FrameBuffer* input)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_linear_to_linear.SetUniform1i("u_tex0", 0);
    pipeline_linear_to_linear.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

    pipeline_linear_to_linear.Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, linear_sampler);

        fs_pass_geom->Draw();

        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_linear_to_linear.Unbind();
}

void MyWindow::PassLinearToSRGB(FrameBuffer* input)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    pipeline_linear_to_srgb.SetUniform1i("u_tex0", 0);
    pipeline_linear_to_srgb.SetUniform2f("u_pixel_size", 1.0f / static_cast<float>(viewport[2]), 1.0f / static_cast<float>(viewport[3]));

    pipeline_linear_to_srgb.Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, input->GetColorTexture());
        glBindSampler(0, linear_sampler);

        fs_pass_geom->Draw();

        glBindSampler(0, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    pipeline_linear_to_srgb.Unbind();
}