#pragma once
#include <glm/gtc/quaternion.hpp>
#include "camera.h"

namespace common::render
{

class SimpleCamera
    : public Camera
{
    static constexpr auto kEx = glm::vec3(1.0f, 0.0f, 0.0f);
    static constexpr auto kEy = glm::vec3(0.0f, 1.0f, 0.0f);
    static constexpr auto kEz = glm::vec3(0.0f, 0.0f, 1.0f);
    static constexpr auto kZero = glm::vec3(0.0f);
    static constexpr auto kForward = glm::vec3(0.0f, 0.0f, -1.0f);
    static constexpr auto kMouseSensitivity = 0.001f;

public:
    SimpleCamera()
        : fov_(glm::radians(45.0f))
        , position_(glm::vec3(0.0f, 0.0f, 0.0f))
        , orientation_(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
        , velocity_(glm::vec3(0.0f, 0.0f, 0.0f))
        , angular_velocity_(glm::vec3(0.0f, 0.0f, 0.0f))
        , l_drag_info_{ DragInfo::State::None, {0.0f, 0.0f} }
        , r_drag_info_{ DragInfo::State::None, {0.0f, 0.0f} }
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

    void LookAt(const glm::vec3& v);

private:
    void translate(const glm::vec3& v){ position_ += orientation_ * v; }
    void translate(float x, float y, float z){ translate(glm::vec3(x, y, z)); }

    void rotate(float angle, const glm::vec3& axis){ orientation_ *= glm::angleAxis(angle, axis); }
    void rotate(float angle, float x, float y, float z){ rotate(angle, glm::vec3(x, y, z)); }

    void rotate(const glm::vec3& euler_angles){ orientation_ *= glm::quat(euler_angles); }
    void rotate(float pitch, float yaw, float roll){ rotate(glm::vec3(pitch, yaw, roll)); }

    void roll(float angle){ rotate(angle, kEz); }
    void pitch(float angle){ rotate(angle, kEx); }
    void yaw(float angle){ rotate(angle, kEy); }

private:
    float fov_;
    glm::vec3 position_;
    glm::quat orientation_;
    glm::vec3 velocity_;
    glm::vec3 angular_velocity_;

    struct DragInfo
    {
        enum class State
        {
            None,
            Enter,
            Move,
            Leave
        };
        State state_;
        glm::vec2 last_pos_;
    };
    DragInfo l_drag_info_;
    DragInfo r_drag_info_;
};

}   // namespace common::render