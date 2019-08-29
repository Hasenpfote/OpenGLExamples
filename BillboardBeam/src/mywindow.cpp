#include <iomanip>
#include <sstream>
#include <filesystem>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "../../common/logger.h"
#include "mywindow.h"

uint64_t nchoosek(uint64_t n, uint64_t k)
{
    assert(n >= k);

    uint64_t r = 1;
    for(uint64_t d = 1; d <= k; ++d)
    {
        r *= n--;
        r /= d;
    }
    return r;
}

float bernstein_basis(uint32_t n, uint32_t k, float t)
{
    // nCk * t^k * (1-t)^(n-k)
    return static_cast<float>(nchoosek(n, k)) * std::pow(t, static_cast<float>(k)) * std::pow(1.0f - t, static_cast<float>(n - k));
}

float derivative_of_bernstein_basis(uint32_t n, uint32_t k, float t)
{
    //
    // b(t) = nCk * t^k * (1-t)^(n-k)
    //
    // f(t) = t^k
    // g(t) = (1-t)^(n-k)
    // b'(t) = nCk * { kt^(k-1) * (1-t)^(n-k) - t^k * (n-k)(1-t)^(n-k-1) }
    //
    float f = std::pow(t, static_cast<float>(k));
    float g = std::pow(1.0f - t, static_cast<float>(n - k));
    float dfdt = static_cast<float>(k) * std::pow(t, static_cast<float>(k - 1));
    float dgdt = -static_cast<float>(n - k) * std::pow(1.0f - t, static_cast<float>(n - k - 1));
    return static_cast<float>(nchoosek(n, k)) * (dfdt * g + f * dgdt);
}

glm::vec3 calc_bezier_point(std::vector<glm::vec3> ctrl_points, float t)
{
    glm::vec3 point(0.0f, 0.0f, 0.0f);
    uint32_t n = ctrl_points.size();
    uint32_t k = 0;
    for(const auto& ctrl_point : ctrl_points)
    {
        auto b = bernstein_basis(n - 1, k, t);
        point += b * ctrl_point;
        k++;
    }
    return point;
}

MyWindow::MyWindow()
{
    LOG_D(__func__);
    theta = 0.0f;
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

        camera.position() = glm::vec3(0.0f, 0.0f, 50.0f);

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
    //
    bb.Initialize();
    //
    ctrl_points.emplace_back(-5.0f,  5.0f, 0.0f);
    ctrl_points.emplace_back(-10.0f, 10.0f, 0.0f);
    ctrl_points.emplace_back(-15.0f, 10.0f, 0.0f);
    ctrl_points.emplace_back(-20.0f,  5.0f, 0.0f);
    ctrl_points.emplace_back(-20.0f, -5.0f, 0.0f);
    ctrl_points.emplace_back(-15.0f,-10.0f, 0.0f);
    ctrl_points.emplace_back( -5.0f, -5.0f, 0.0f);
    ctrl_points.emplace_back(  5.0f,  5.0f, 0.0f);
    ctrl_points.emplace_back( 10.0f, 10.0f, 0.0f);
    ctrl_points.emplace_back( 15.0f, 10.0f, 0.0f);
    ctrl_points.emplace_back( 20.0f,  5.0f, 0.0f);
    ctrl_points.emplace_back( 20.0f, -5.0f, 0.0f);
    ctrl_points.emplace_back( 15.0f,-10.0f, 0.0f);
    ctrl_points.emplace_back( 10.0f,-10.0f, 0.0f);
    ctrl_points.emplace_back(  5.0f, -5.0f, 0.0f);
    ctrl_points.emplace_back( -5.0f,  5.0f, 0.0f);
    t = 0.0f;
}

void MyWindow::Cleanup()
{
}

void MyWindow::OnKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window::OnKey(window, key, scancode, action, mods);

    System::GetMutableInstance().GetCamera().OnKey(key, scancode, action, mods);

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
    System::GetMutableInstance().GetCamera().OnMouseWheel(xoffset, yoffset);
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

    auto& camera = System::GetMutableInstance().GetCamera();
    camera.viewport().size() = glm::vec2(width, height);
}

void MyWindow::OnUpdate(double dt)
{
    auto& camera = System::GetMutableInstance().GetCamera();
    camera.Update(dt);

    theta += 10.0f * dt;
    theta = std::fmodf(theta, 360.0f);

    t += static_cast<float>(dt);
    if(t > 1.0f)
        t = 0.0f;
    auto point = calc_bezier_point(ctrl_points, t);
    points.push_front(point);
    if(points.size() >= 10){
        points.pop_back();
    }
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

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    glBlendFunc(GL_ONE, GL_ONE);
    DrawCube();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    DrawPoints();
    DrawLines();

    DrawCurve();

    glDisable(GL_FRAMEBUFFER_SRGB);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

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

        text->BeginRendering();
        {
            text->DrawString(std::move(oss.str()), 0.0f, 0.0f, 0.5f);
        }
        text->EndRendering();

        //oss.str("");
        //oss.clear(std::stringstream::goodbit);
    }
}

void MyWindow::DrawCube()
{
    bb.UpdateMatrices(glm::rotate(glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f)));

    constexpr float size = 0.1f;

    bb.Draw(glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(10.0f, -10.0f, -10.0f), size);
    bb.Draw(glm::vec3(-10.0f, -10.0f, 10.0f), glm::vec3(10.0f, -10.0f, 10.0f), size);
    bb.Draw(glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(-10.0f, -10.0f, 10.0f), size);
    bb.Draw(glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(10.0f, -10.0f, 10.0f), size);

    bb.Draw(glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec3(-10.0f, 10.0f, -10.0f), size);
    bb.Draw(glm::vec3(-10.0f, -10.0f, 10.0f), glm::vec3(-10.0f, 10.0f, 10.0f), size);
    bb.Draw(glm::vec3(10.0f, -10.0f, -10.0f), glm::vec3(10.0f, 10.0f, -10.0f), size);
    bb.Draw(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(10.0f, 10.0f, 10.0f), size);

    bb.Draw(glm::vec3(-10.0f, 10.0f, -10.0f), glm::vec3(10.0f, 10.0f, -10.0f), size);
    bb.Draw(glm::vec3(-10.0f, 10.0f, 10.0f), glm::vec3(10.0f, 10.0f, 10.0f), size);
    bb.Draw(glm::vec3(-10.0f, 10.0f, -10.0f), glm::vec3(-10.0f, 10.0f, 10.0f), size);
    bb.Draw(glm::vec3(10.0f, 10.0f, -10.0f), glm::vec3(10.0f, 10.0f, 10.0f), size);
}

void MyWindow::DrawLines()
{
    bb.UpdateMatrices(glm::mat4(1.0f));
    bb.Draw(glm::vec3(-20.0f, 0.0f, -20.0f), glm::vec3(-20.0f, 0.0f, 20.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.5f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);
    bb.Draw(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(0.0f, 0.0f, 20.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.5f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 1.0f);
    bb.Draw(glm::vec3(20.0f, 0.0f, -20.0f), glm::vec3(20.0f, 0.0f, 20.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.5f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 1.0f);
}

void MyWindow::DrawPoints()
{
    bb.UpdateMatrices(glm::mat4(1.0f));
    bb.Draw(glm::vec3(-20.0f, 5.0f,  0.0f), glm::vec3(-20.0f, 5.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);
    bb.Draw(glm::vec3(  0.0f, 5.0f,  0.0f), glm::vec3(  0.0f, 5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.5f), 1.0f);
    bb.Draw(glm::vec3( 20.0f, 5.0f,  0.0f), glm::vec3( 20.0f, 5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.5f), 1.0f);
}

void MyWindow::DrawCurve()
{
    auto size = points.size();
    if(size < 2)
        return;

    bb.UpdateMatrices(glm::mat4(1.0f));

    float delta = 1.0f / static_cast<float>(size - 1);
    float alpha = 1.0f;
    for(auto i = 0; i < size-1; i++)
    {
        auto p0 = points[i];
        auto p1 = points[i+1];
        bb.Draw(p0, p1, glm::vec4(1.0f, 1.0f, 0.0f, alpha), glm::vec4(1.0f, 1.0f, 0.0f, alpha - delta), 0.25f);
        alpha -= delta;
    }
}