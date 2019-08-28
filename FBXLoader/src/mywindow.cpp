#include <iomanip>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../../common/logger.h"
#include "mywindow.h"

static void render_basis(float length)
{
    glBegin(GL_LINES);
        glColor3f(1.0f, 0.5f, 0.5f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(length, 0.0f, 0.0f);

        glColor3f(0.5f, 1.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, length, 0.0f);

        glColor3f(0.5f, 0.5f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, length);
    glEnd();
}

MyWindow::MyWindow()
{
    LOG_D(__func__);
#ifdef ENABLE_FBX_TEST
    rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
#endif
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

        camera.position() = glm::vec3(0.0f, 150.0f, 300.0f);

        camera.Update(0.0);

        common_matrices.view = camera.view();
        common_matrices.projection = camera.proj();
    }
    // load shader.
    {
        std::filesystem::path dirpath("assets/shaders");
        auto& rm = System::GetMutableInstance().GetResourceManager();
        rm.AddResourcesFromDirectory<Program>(dirpath, false);
    }

    // 共通の変換用行列群
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    //
    const std::string model_root("assets/models");
    // load model.
    {
        const std::string fbx_filepath(model_root + "/unitychan.fbx");
        auto fbx_model = std::unique_ptr<fbxloader::Model>(new fbxloader::Model());
        fbx_model->Load(fbx_filepath);
        model.Setup(*fbx_model, ubo);
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
}

void MyWindow::Cleanup()
{
    if(glIsBuffer(ubo))
        glDeleteBuffers(1, &ubo);
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
    else
    if(key == GLFW_KEY_J && action == GLFW_PRESS){
        is_draw_joints_enabled = !is_draw_joints_enabled;
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

    common_matrices.view = camera.view();
    common_matrices.projection = camera.proj();

    model.Update(dt);
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

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(common_matrices.projection));

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(common_matrices.view));

    glPushMatrix();
    render_basis(100.0f);
    glPopMatrix();

    glBufferData(GL_UNIFORM_BUFFER, sizeof(CommonMatrices), &common_matrices, GL_DYNAMIC_DRAW);

    auto matWorld = glm::mat4(1.0f);
#if 0
    CMatrix4 matInvView = CMatrix4::Inverse(matView);
    matInvView.m14 = matInvView.m24 = matInvView.m34 = 0.0f;
    auto qr = arcball.GetRotationQuaternion();
    auto aa = qr.ToAxisAngle();
    auto axis = matInvView * aa.axis;
    rot = Quaternion::RotationAxis(axis, aa.angle) * rot;
    matWorld = rot.ToRotationCMatrix() * matWorld;
#endif
    glEnable(GL_FRAMEBUFFER_SRGB);

    glDisable(GL_BLEND);
    model.DrawOpaqueMeshes(matWorld);

    // TODO: 内部は未ソート
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    model.DrawTransparentMeshes(matWorld);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // スケルトンの表示
    if(is_draw_joints_enabled){
        glPushMatrix();
        glMultMatrixf(glm::value_ptr(matWorld));
        model.DrawSkeleton();
        glPopMatrix();
    }

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
    oss << width << "x" << height;
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    GLboolean ms;
    glGetBooleanv(GL_MULTISAMPLE, &ms);
    oss << "MultiSample:" << ((ms == GL_TRUE) ? "On" : "Off") << "(Toggle MultiSample: m)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    oss << "DrawJoints:" << ((is_draw_joints_enabled) ? "On" : "Off") << "(Toggle DrawJoints: j)";
    text_lines.push_back(oss.str());
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    DrawTextLines(text_lines);
}

void MyWindow::DrawTextLines(const std::vector<std::string>& text_lines)
{
    if(text_lines.empty())
        return;

    auto metrics = text->GetFont().GetFontMetrics();
    auto line_height = static_cast<float>(metrics.GetLineHeight());
    const float scale = 0.5f;
    const float fh = line_height * scale;

    static const glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    text->SetColor(glm::value_ptr(color));

    text->BeginRendering();
    int line_no = 1;
    for(const auto& text_line : text_lines)
    {
        text->DrawString(text_line, 0.0f, fh * static_cast<float>(line_no), scale);
        line_no++;
    }

    text->EndRendering();
}