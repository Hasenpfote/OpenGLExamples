#include <GLFW/glfw3.h>
#include <hasenpfote/math/utility.h>
#include <hasenpfote/math/vector4.h>
#include "custom_camera.h"

using namespace hasenpfote::math;

const Vector3 CustomCamera::FORWARD = Vector3(0.0f, 0.0f, -1.0f);
const Vector3 CustomCamera::RIGHT = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 CustomCamera::UP = Vector3(0.0f, 1.0f, 0.0f);

const float CustomCamera::_35mm_film_diagonal = std::sqrtf(1872.0f);    // sqrt(36^2 + 24^2)

CustomCamera::CustomCamera()
{
    position = Vector3::ZERO;
    rotation = Quaternion::IDENTITY;
    velocity = Vector3::ZERO;

    effective_sensor_width = sensor_width;
    effective_sensor_height = sensor_height;
    effective_sensor_diagonal = std::sqrtf(sensor_width * sensor_width + sensor_height * sensor_height);
    crop_factor = ComputeCropFactor(effective_sensor_diagonal);
    focal_length = 50.0f / crop_factor;
    zoom_magnification = 1.0f;

    near = 1.0f;
    far = 100.0f;
    is_dirty_proj = true;

    camera.GetViewport().Set(0, 0, 0, 0);

    motion = Motion::Stop;
}

void CustomCamera::SetTargetPosition(const hasenpfote::math::Vector3& target)
{
    Vector3 dst = target - position;
    if(dst.Magnitude() > 0.0f){
        dst.Normalize();
        auto src = GetForward();
        if (Vector3::DotProduct(src, dst) < 1.0f){
            rotation = Quaternion::RotationShortestArc(src, dst);
        }
    }
}

Vector3 CustomCamera::GetForward() const
{
    return rotation.Rotate(FORWARD);
}

Vector3 CustomCamera::GetRight() const
{
    return rotation.Rotate(RIGHT);
}

Vector3 CustomCamera::GetUp() const
{
    return rotation.Rotate(UP);
}

void CustomCamera::Set35mmEquivalentFocalLength(float focal_length)
{
    SetFocalLength(focal_length / crop_factor);
}

float CustomCamera::Get35mmEquivalentFocalLength() const
{
    return GetFocalLength() * crop_factor;
}

void CustomCamera::SetFocalLength(float focal_length)
{
    this->focal_length = focal_length;
    zoom_magnification = 1.0f; // reset
    const float tele_end = ComputeFocalLength(effective_sensor_diagonal, to_radians(1.0f));
    max_zoom_magnification = ComputeMaxZoomMagnification(tele_end, focal_length);
    is_dirty_proj = true;
}

float CustomCamera::GetFocalLength() const
{
    return focal_length * zoom_magnification;
}

float CustomCamera::GetAngleOfView(AngleOfView aov) const
{
    switch(aov){
    case AngleOfView::Horizontal:
        return ComputeAngleOfView(effective_sensor_width, GetFocalLength());
    case AngleOfView::Vertical:
        return ComputeAngleOfView(effective_sensor_height, GetFocalLength());
    case AngleOfView::Diagonal:
        return ComputeAngleOfView(effective_sensor_diagonal, GetFocalLength());
    default:
        return 0.0f;
    }
}

#if 0
void CustomCamera::SetFov(float fov)
{
    this->fov = fov;
    focal_length = ComputeFocalLength(film_diameter, fov);
    zoom_magnification = 1.0f;  // reset
    const float tele_end = ComputeFocalLength(film_diameter, to_radians(1.0f));
    max_zoom_magnification = ComputeMaxZoomMagnification(tele_end, focal_length);
    is_dirty_proj = true;
}
#endif

void CustomCamera::SetZoomMagnification(float zoom_magnification)
{
    if(zoom_magnification < 1.0f){
        zoom_magnification = 1.0f;
    }
    else
    if(zoom_magnification > max_zoom_magnification){
        zoom_magnification = max_zoom_magnification;
    }
    this->zoom_magnification = zoom_magnification;
    is_dirty_proj = true;
}

void CustomCamera::SetClippingPlane(float near, float far)
{
    this->near = near;
    this->far = far;
    is_dirty_proj = true;
}

void CustomCamera::SetViewport(int x, int y, int width, int height)
{
    camera.GetViewport().SetPosition(x, y);
    SetViewportSize(width, height);
}

void CustomCamera::SetViewportSize(int width, int height)
{
    auto& vp = camera.GetViewport();
    if(vp.GetWidth() == width && vp.GetHeight() == height)
        return;

    vp.SetSize(width, height);
    const auto aspect = vp.GetAspectRatio();

#ifndef ENABLE_MULTI_ASPECT
    ComputeEffectiveSensorSize(effective_sensor_width, effective_sensor_height, sensor_width, aspect);
#else
    ComputeEffectiveSensorSizeFromImageCircle(effective_sensor_width, effective_sensor_height, _35mm_film_diagonal, aspect);
#endif
    effective_sensor_diagonal = std::sqrt(effective_sensor_width * effective_sensor_width + effective_sensor_height * effective_sensor_height);
    crop_factor = ComputeCropFactor(effective_sensor_diagonal);
    SetFocalLength(focal_length);
}

void CustomCamera::Update(double dt)
{
    const Vector3 vel = rotation.Rotate(velocity);
    position += vel * dt;

    CMatrix4 m = rotation.ToRotationCMatrix();
    m.SetColumn(3, Vector4(position, 1.0f));
    camera.SetViewMatrix(CMatrix4::Inverse(m));

    if(is_dirty_proj){
        camera.SetProjectionMatrix(CMatrix4::Perspective(ComputeAngleOfView(effective_sensor_height, GetFocalLength()), camera.GetViewport().GetAspectRatio(), near, far));
        is_dirty_proj = false;
    }
}

void CustomCamera::OnKey(int key, int scancode, int action, int mods)
{
    switch (key) {
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
#if 0
    case GLFW_KEY_P:
        if (action == GLFW_PRESS) {
            mode = (mode == ProjectionMode::Perspective) ? ProjectionMode::Orthographic : ProjectionMode::Perspective;
        }
        break;
#endif
    default:
        break;
    }
}

void CustomCamera::OnMouseMove(double xpos, double ypos)
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

void CustomCamera::OnMouseButton(int button, int action, int mods)
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

void CustomCamera::OnMouseWheel(double xoffset, double yoffset)
{
    SetZoomMagnification(GetZoomMagnification() + static_cast<float>(yoffset));
}

#ifndef ENABLE_MULTI_ASPECT
void CustomCamera::ComputeEffectiveSensorSize(float& effective_width, float& effective_height, float reference_width, float image_aspect)
{
    effective_width = reference_width;
    effective_height = reference_width / image_aspect;
}
#else
void CustomCamera::ComputeEffectiveSensorSizeFromImageCircle(float& effective_width, float& effective_height, float circle_diameter, float image_aspect)
{
    const float hypotenuse = std::sqrtf(image_aspect * image_aspect + 1);  // sqrt(aspect^2 + 1^2)
    effective_width = circle_diameter * image_aspect / hypotenuse;
    effective_height = circle_diameter / hypotenuse;
}
#endif

float CustomCamera::ComputeCropFactor(float sensor_diagonal)
{
    return _35mm_film_diagonal / sensor_diagonal;
}

float CustomCamera::ComputeFocalLength(float size, float aov)
{
    return 0.5f * size / std::tanf(aov * 0.5f);
}

float CustomCamera::ComputeAngleOfView(float size, float focal_length)
{
    return 2.0f * std::atanf(0.5f * size / focal_length);
}

float CustomCamera::ComputeMaxZoomMagnification(float tele_end, float wide_end)
{
    // max zoom mag = focal lenght[tele end] / focal length[wide end]
    return tele_end / wide_end;
}

Vector3 CustomCamera::ToSemiSphere(float x, float y)
{
    const Viewport& vp = camera.GetViewport();
    const float aspect = vp.GetAspectRatio();

    Vector3 result;
    result.SetX(remap(x, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()), -aspect, aspect));
    result.SetY(remap(y, static_cast<float>(vp.GetPositionY()), static_cast<float>(vp.GetHeight()), 1.0f, -1.0f));

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