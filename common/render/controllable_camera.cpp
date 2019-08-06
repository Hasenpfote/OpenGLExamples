#include <GLFW/glfw3.h>
#include <hasenpfote/math/utils.h>
#include <hasenpfote/math/vector4.h>
#include "controllable_camera.h"

namespace common::render
{

using namespace hasenpfote::math;

const Vector3 ControllableCamera::FORWARD = Vector3(0.0f, 0.0f, -1.0f);
const Vector3 ControllableCamera::RIGHT = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 ControllableCamera::UP = Vector3(0.0f, 1.0f, 0.0f);

ControllableCamera::ControllableCamera()
{
    position = Vector3::ZERO;
    rotation = Quaternion::IDENTITY;
    velocity = Vector3::ZERO;
    motion = Motion::Stop;

    near = 1.0f;
    far = 100.0f;
    fovy = ConvertDegreesToRadians(45.0f);
    zoom = far;
    mode = ProjectionMode::Perspective;

    GetViewport().Set(0, 0, 0, 0);
}

void ControllableCamera::Update(double dt)
{
    const Vector3 vel = rotation.Rotate(velocity);
    position += vel * dt;

    CMatrix4 m = rotation.ToRotationCMatrix();
    m.SetColumn(3, Vector4(position, 1.0f));

    SetViewMatrix(CMatrix4::Inverse(m));

    if(mode == ProjectionMode::Perspective){
        SetProjectionMatrix(CMatrix4::Perspective(fovy, GetViewport().GetAspectRatio(), near, far));
    }
    else{
        const float cot = 1.0f / std::tanf(fovy * 0.5f);
        const float t = 1.0f / cot * 0.5f;
        const float b = -t;
        const float r = GetViewport().GetAspectRatio() / cot * 0.5f;
        const float l = -r;
        SetProjectionMatrix(CMatrix4::Ortho(t * zoom, b * zoom, l * zoom, r * zoom, near, far));
    }
}

Vector3 ControllableCamera::GetForward() const
{
    return rotation.Rotate(FORWARD);
}

Vector3 ControllableCamera::GetRight() const
{
    return rotation.Rotate(RIGHT);
}

Vector3 ControllableCamera::GetUp() const
{
    return rotation.Rotate(UP);
}

void ControllableCamera::SetTargetPosition(const hasenpfote::math::Vector3& target)
{
    Vector3 dst = target - position;
    if(dst.Magnitude() > 0.0f){
        dst.Normalize();
        auto src = GetForward();
        if(Vector3::DotProduct(src, dst) < 1.0f){
            rotation = Quaternion::RotationShortestArc(src, dst);
        }
    }
}

void ControllableCamera::OnKey(int key, int scancode, int action, int mods)
{
    switch(key){
    case GLFW_KEY_W:
        if(action == GLFW_PRESS || action == GLFW_REPEAT){
            velocity += FORWARD;
        }
        else
        if(action == GLFW_RELEASE){
            velocity = Vector3::ZERO;
        }
        break;
    case GLFW_KEY_S:
        if(action == GLFW_PRESS || action == GLFW_REPEAT){
            velocity -= FORWARD;
        }
        else
        if(action == GLFW_RELEASE){
            velocity = Vector3::ZERO;
        }
        break;
    case GLFW_KEY_D:
        if(action == GLFW_PRESS || action == GLFW_REPEAT){
            velocity += RIGHT;
        }
        else
        if(action == GLFW_RELEASE){
            velocity = Vector3::ZERO;
        }
        break;
    case GLFW_KEY_A:
        if(action == GLFW_PRESS || action == GLFW_REPEAT){
            velocity -= RIGHT;
        }
        else
        if(action == GLFW_RELEASE){
            velocity = Vector3::ZERO;
        }
        break;
    case GLFW_KEY_E:
        if(action == GLFW_PRESS || action == GLFW_REPEAT){
            velocity += UP;
        }
        else
        if(action == GLFW_RELEASE){
            velocity = Vector3::ZERO;
        }
        break;
    case GLFW_KEY_Q:
        if(action == GLFW_PRESS || action == GLFW_REPEAT){
            velocity -= UP;
        }
        else
        if(action == GLFW_RELEASE){
            velocity = Vector3::ZERO;
        }
        break;
    case GLFW_KEY_P:
        if(action == GLFW_PRESS){
            mode = (mode == ProjectionMode::Perspective) ? ProjectionMode::Orthographic : ProjectionMode::Perspective;
        }
        break;
    default:
        break;
    }
}

void ControllableCamera::OnMouseMove(double xpos, double ypos)
{
    if(motion == Motion::Start){
        prev = ToSemiSphere(static_cast<float>(xpos), static_cast<float>(ypos));
        motion = Motion::Tracking;
    }
    else
    if(motion == Motion::Tracking){
        current = ToSemiSphere(static_cast<float>(xpos), static_cast<float>(ypos));
#if 0
        auto diff = current - prev;
        auto qp = Quaternion::RotationAxis(Vector3(1.0f, 0.0f, 0.0f), diff.y);
        auto qy = Quaternion::RotationAxis(Vector3(0.0f, 1.0f, 0.0f), diff.x);
        rotation = rotation * qp * qy;
#else
        if(Vector3::DotProduct(prev, current) < 1.0f){
            auto rot = Quaternion::RotationShortestArc(prev, current);
            rotation = rotation * rot;
        }
#endif
        prev = current;
    }
}

void ControllableCamera::OnMouseButton(int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT){
        if(action == GLFW_PRESS){
            motion = Motion::Start;
        }
        else
        if(action == GLFW_RELEASE){
            motion = Motion::Stop;
        }
    }
}

void ControllableCamera::OnMouseWheel(double xoffset, double yoffset)
{
    if(mode == ProjectionMode::Perspective){
        fovy += ConvertDegreesToRadians(static_cast<float>(yoffset));
        fovy = Clamp(fovy, ConvertDegreesToRadians(1.0f), ConvertDegreesToRadians(90.0f));
    }
    else{
        zoom += static_cast<float>(yoffset);
        zoom = Clamp(zoom, near, far);
    }
}

Vector3 ControllableCamera::ToSemiSphere(float x, float y)
{
    const Viewport& vp = GetViewport();
    const float aspect = vp.GetAspectRatio();

    Vector3 result;
    result.SetX(Remap(x, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()), -aspect, aspect));
    result.SetY(Remap(y, static_cast<float>(vp.GetPositionY()), static_cast<float>(vp.GetHeight()), 1.0f, -1.0f));

    const float d = result.GetX() * result.GetX() + result.GetY() * result.GetY();
    if(d <= 1.0f){
        result.SetZ(std::sqrtf(1.0f - d));
    }
    else{
        result.SetZ(0.0f);
        result.Normalize();
    }
    return result;
}

}   // namespace common::render