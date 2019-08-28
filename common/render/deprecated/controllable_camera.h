#pragma once
#include "camera.h"

namespace common::render
{

class ControllableCamera final : public Camera
{
public:
    enum class ProjectionMode
    {
        Perspective,
        Orthographic
    };

private:
    static const hasenpfote::math::Vector3 FORWARD;
    static const hasenpfote::math::Vector3 RIGHT;
    static const hasenpfote::math::Vector3 UP;

    hasenpfote::math::Vector3 position;
    hasenpfote::math::Quaternion rotation;
    hasenpfote::math::Vector3 velocity;

    ProjectionMode mode;
    float near, far;
    float fovy;
    float zoom;

    enum class Motion
    {
        Stop,
        Start,
        Tracking
    } motion;

    hasenpfote::math::Vector3 prev;
    hasenpfote::math::Vector3 current;

public:
    ControllableCamera();
    ~ControllableCamera() = default;

    void Update(double dt);

    // view
    void SetPosition(const hasenpfote::math::Vector3& position) { this->position = position; }
    hasenpfote::math::Vector3 GetPosition() { return position; }

    hasenpfote::math::Vector3 GetForward() const;
    hasenpfote::math::Vector3 GetRight() const;
    hasenpfote::math::Vector3 GetUp() const;

    void SetTargetPosition(const hasenpfote::math::Vector3& target);

    // projection
    void SetProjectionMode(ProjectionMode mode) { this->mode = mode; }
    ProjectionMode GetProjectionMode() { return mode; }
    void SetFieldOfView(float fovy) { this->fovy = fovy; }
    void SetNearClippingPlane(float value) { near = value; }
    void SetFarClippingPlane(float value) { far = value; }
    void SetClippingPlane(float near, float far)
    {
        this->near = near;
        this->far = far;
    }

    // motion
    void OnKey(int key, int scancode, int action, int mods);
    void OnMouseMove(double xpos, double ypos);
    void OnMouseButton(int button, int action, int mods);
    void OnMouseWheel(double xoffset, double yoffset);

private:
    hasenpfote::math::Vector3 ToSemiSphere(float x, float y);
};

}   // namespace common::render