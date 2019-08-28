#pragma once
#include <glm/gtc/quaternion.hpp>
#include "camera.h"

namespace common::render
{

class SimpleCamera
    : public Camera
{
public:
    SimpleCamera()
        : position_(glm::vec3(0.0f, 0.0f, 0.0f))
        , orientation_(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
        , velocity_(glm::vec3(0.0f, 0.0f, 0.0f))
        , angular_velocity_(glm::vec3(0.0f, 0.0f, 0.0f))
        , fov_(glm::radians(45.0f))
        , tracking_state_(TrackingState::Idling)
    {}
    virtual ~SimpleCamera() = default;
    SimpleCamera(const SimpleCamera&) = default;
    SimpleCamera& operator = (const SimpleCamera&) = default;
    SimpleCamera(SimpleCamera&&) = default;
    SimpleCamera& operator = (SimpleCamera&&) = default;

    void OnKey(int key, int scancode, int action, int mods);

    void OnMouseMove(double xpos, double ypos);

    void OnMouseButton(int button, int action, int mods);

    void OnMouseWheel(double xoffset, double yoffset);

    void Update(double dt);

    const glm::vec3& position() const noexcept { return position_; }
    glm::vec3& position() noexcept { return position_; }

    const glm::quat& orientation() const noexcept { return orientation_; }
    glm::quat& orientation() noexcept { return orientation_; }

    const float& fov() const noexcept { return fov_; }
    float& fov() noexcept { return fov_; }

private:
    void translate(const glm::vec3& v) { position_ += orientation_ * v; }
    void translate(float x, float y, float z){ translate(glm::vec3(x, y, z)); }

    void rotate(float angle, const glm::vec3& axis){ orientation_ *= glm::angleAxis(angle, axis); }
    void rotate(float angle, float x, float y, float z){ rotate(angle, glm::vec3(x, y, z)); }

    void roll(float angle){ rotate(angle, 0.0f, 0.0f, 1.0f); }
    void pitch(float angle){ rotate(angle, 1.0f, 0.0f, 0.0f); }
    void yaw(float angle){ rotate(angle, 0.0f, 1.0f, 0.0f); }

private:
    glm::vec3 position_;
    glm::quat orientation_;
    glm::vec3 velocity_;
    glm::vec3 angular_velocity_;
    float fov_;

    enum class TrackingState
    {
        Idling,
        Starting,
        Tracking,
        Stopping,
    } tracking_state_;

};

}   // namespace common::render