#include <iomanip>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../../common/logger.h"
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

    glClearColor(160.0f / 255.0f, 216.0f / 255.0f, 239.0f / 255.0f, 1.0);

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

        camera.position() = glm::vec3(0.0f, 0.0f, 10.0f);

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
    quad.Initialize();
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

    glEnable(GL_FRAMEBUFFER_SRGB);
    {
        quad.Draw();
    }
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

        text->BeginRendering();
        {
            text->DrawString(std::move(oss.str()), 0.0f, 0.0f, 0.5f);
        }
        text->EndRendering();

        //oss.str("");
        //oss.clear(std::stringstream::goodbit);
    }
}