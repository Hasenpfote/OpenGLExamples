#include <iomanip>
#include <sstream>
#include <GL/glew.h>
#include <hasenpfote/assert.h>
#include <hasenpfote//math/utils.h>
#include <hasenpfote/math/vector3.h>
#include <hasenpfote/math/vector4.h>
#include <hasenpfote/math/cmatrix4.h>
#include <hasenpfote/math/axis_angle.h>
#include "../../Common/system.h"
#include "../../Common/logger.h"
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
    rot = Quaternion::IDENTITY;
#endif
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

    glClearColor(160.0f / 255.0f, 216.0f / 255.0f, 239.0f / 255.0f, 1.0);

    //
    auto window = GetWindow();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    auto& camera = System::GetMutableInstance().GetCamera();

    camera.SetViewport(0, 0, width, height);
    //camera.Set35mmEquivalentFocalLength(50.0f);
    camera.SetClippingPlane(1.0f, 1000.0f);

    camera.SetPosition(Vector3(0.0f, 150.0f, 300.0f));
    camera.SetTargetPosition(Vector3(0.0f, 150.0f, 0.0f));

    camera.Update(0.0f);
    common_matrices.view = camera.GetViewMatrix();
    common_matrices.projection = camera.GetProjectionMatrix();

    // load shader.
    {
        auto& man = System::GetMutableInstance().GetShaderManager();
        std::filesystem::path directory("assets/shaders");
        man.LoadShaderPrograms(directory);
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
}

void MyWindow::Cleanup()
{
    if(glIsBuffer(ubo))
        glDeleteBuffers(1, &ubo);
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
    System::GetMutableInstance().GetCamera().SetViewportSize(width, height);
}

void MyWindow::OnUpdate(double dt)
{
    auto& camera = System::GetMutableInstance().GetCamera();
    camera.Update(dt);

    common_matrices.view = camera.GetViewMatrix();
    common_matrices.projection = camera.GetProjectionMatrix();

    model.Update(dt);
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

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(static_cast<GLfloat*>(common_matrices.projection));

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(static_cast<GLfloat*>(common_matrices.view));

    glPushMatrix();
    render_basis(100.0f);
    glPopMatrix();

    glBufferData(GL_UNIFORM_BUFFER, sizeof(CommonMatrices), &common_matrices, GL_DYNAMIC_DRAW);

    CMatrix4 matWorld = CMatrix4::IDENTITY;
#if 0
    CMatrix4 matInvView = CMatrix4::Inverse(matView);
    matInvView.m14 = matInvView.m24 = matInvView.m34 = 0.0f;
    auto qr = arcball.GetRotationQuaternion();
    auto aa = qr.ToAxisAngle();
    auto axis = matInvView * aa.axis;
    rot = Quaternion::RotationAxis(axis, aa.angle) * rot;
    matWorld = rot.ToRotationCMatrix() * matWorld;
#endif

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
        glMultMatrixf(static_cast<GLfloat*>(matWorld));
        model.DrawSkeleton();
        glPopMatrix();
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

    oss << "DrawJoints:" << ((is_draw_joints_enabled) ? "On" : "Off") << "(Toggle DrawJoints: j)";
    text->DrawString(oss.str(), 0.0f, fh * 7.0f, scale);
    oss.str("");
    oss.clear(std::stringstream::goodbit);

    text->EndRendering();
}