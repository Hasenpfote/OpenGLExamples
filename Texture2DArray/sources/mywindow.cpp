#include <iomanip>
#include <sstream>
#include <GL/glew.h>
#include <hasenpfote/assert.h>
#include <hasenpfote/math/utils.h>
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

    glClearColor(0.75f, 0.75f, 0.75f, 1.0);

    //
    auto window = GetWindow();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    auto& camera = System::GetMutableInstance().GetCamera();

    camera.SetViewport(0, 0, width, height);
    //camera.Set35mmEquivalentFocalLength(50.0f);
    camera.SetClippingPlane(1.0f, 10000.0f);

    camera.SetPosition(Vector3(0.0f, 0.0f, 50.0f));
    camera.SetTargetPosition(Vector3(0.0f, 0.0f, 0.0f));

    camera.Update(0.0f);

    // load shader.
    {
        auto& man = System::GetMutableInstance().GetShaderManager();
        std::filesystem::path directory("assets/shaders");
        man.LoadShaderPrograms(directory);
    }
    // load texture.
    {
#if 0
        std::tr2::sys::path directory("assets/textures");
        auto& man = System::GetMutableInstance().GetTextureManager();
        man.LoadTextures(directory);
#endif
    }
    // generate font.
    {
        //std::tr2::sys::path fontpath = "assets/fonts/mp1mm16tir.fnt";
        std::filesystem::path fontpath = "assets/fonts/test3.fnt";
        auto font = std::make_shared<text::Font>(fontpath);
        text = std::make_unique<SDFText>(font, std::make_shared<SDFTextRenderer>());
        text->SetSmoothness(smoothness);
    }
    //
    quad.Initialize();
}

void MyWindow::Cleanup()
{
}

void MyWindow::OnKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window::OnKey(window, key, scancode, action, mods);

    auto& camera = System::GetMutableInstance().GetCamera();
    camera.OnKey(key, scancode, action, mods);

    if(key == GLFW_KEY_M && action == GLFW_PRESS){
        GLboolean ms;
        glGetBooleanv(GL_MULTISAMPLE, &ms);
        if (ms == GL_TRUE) {
            glDisable(GL_MULTISAMPLE);
        }
        else {
            glEnable(GL_MULTISAMPLE);
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
#if 0
    System::GetMutableInstance().GetCamera().OnMouseWheel(xoffset, yoffset);
#else
    smoothness = hasenpfote::math::Clamp(smoothness + static_cast<float>(yoffset) * 0.01f, 0.0f, 1.0f);
    text->SetSmoothness(smoothness);
#endif
}

void MyWindow::OnResizeFramebuffer(GLFWwindow* window, int width, int height)
{
    if(HasIconified())
        return;
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

    glEnable(GL_FRAMEBUFFER_SRGB);
    {
        quad.Draw();
    }
    glDisable(GL_FRAMEBUFFER_SRGB);

    // 情報の表示
    auto metrics = text->GetFont().GetFontMetrics();
    auto ascent = static_cast<float>(metrics.GetAscent());
    auto descent = static_cast<float>(metrics.GetDescent());
    auto line_gap = static_cast<float>(metrics.GetLineGap());
    auto line_height = static_cast<float>(metrics.GetLineHeight());

    const float scale = 1.0f;
    const float lh = line_height * scale;

    static const Vector4 color(1.0f, 1.0f, 1.0f, 1.0f);
    text->SetColor(static_cast<const GLfloat*>(color));
    static const Vector4 color2(0.0f, 0.0f, 0.0f, 1.0f);
    text->SetOutlineColor(static_cast<const GLfloat*>(color2));

    text->BeginRendering();

    std::ostringstream oss;
    oss << "FPS:UPS=";
    oss << std::fixed << std::setprecision(2);
    oss << GetFPS() << ":" << GetUPS();
    text->DrawString(oss.str(), 0.0f, lh, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Screen size:";
    oss << camera.GetViewport().GetWidth() << "x" << camera.GetViewport().GetHeight();
    text->DrawString(oss.str(), 0.0f, lh * 2, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);
#if 0
    oss << "Aov D=" << to_degrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Diagonal));
    oss << " H=" << to_degrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Horizontal));
    oss << " V=" << to_degrees(camera.GetAngleOfView(CustomCamera::AngleOfView::Vertical));
    current_text->DrawString(oss.str(), 0.0f, fh * 2.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Focal length=" << camera.GetFocalLength() << " (35mm=" << camera.Get35mmEquivalentFocalLength() << ")";
    current_text->DrawString(oss.str(), 0.0f, fh * 3.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "Zoom=x" << camera.GetZoomMagnification();
    current_text->DrawString(oss.str(), 0.0f, fh * 4.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    GLboolean ms;
    glGetBooleanv(GL_MULTISAMPLE, &ms);
    oss << "MultiSample:" << ((ms == GL_TRUE) ? "On" : "Off") << "(Toggle MultiSample: m)";
    current_text->DrawString(oss.str(), 0.0f, fh * 5.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);
#else
    oss << "Smoothness=" << smoothness;
    text->DrawString(oss.str(), 0.0f, lh * 3.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    text->DrawString("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0.0f, lh * 4.0f, scale);
    text->DrawString("abcdefghijklmnopqrstuvwxyz.", 0.0f, lh * 5.0f, scale);
    text->DrawString("1234567890", 0.0f, lh * 6.0f, scale);
    text->DrawString("\"!`?'.,;:()[]{}<>|/@\\^$-%+=#_&~*", 0.0f, lh * 7.0f, scale);

    auto w = static_cast<float>(width - metrics.MeasureWidth(u"AVT: あ")) * scale;
    text->DrawString(u"AVT: あ", w, lh * 8.0f, scale);

    text->DrawString("\"!`?'.,;:()[]{}<>|/@\\^$-%+=#_&~*", 0.0f, height - descent - line_gap - line_height * scale , scale);
    text->DrawString("\"!`?'.,;:()[]{}<>|/@\\^$-%+=#_&~*", 0.0f, height - descent - line_gap, scale);

#endif
    text->EndRendering();
}