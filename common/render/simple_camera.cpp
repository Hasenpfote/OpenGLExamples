#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/gtx/quaternion.hpp>
#include "../logger.h"
#include "simple_camera.h"

namespace common::render
{

void SimpleCamera::OnKey(int key, int scancode, int action, int mods)
{
    switch(key)
    {
    case GLFW_KEY_W:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ -= kEz;
        else if(action == GLFW_RELEASE)
            velocity_ = kZero;
        break;
    case GLFW_KEY_S:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ += kEz;
        else if (action == GLFW_RELEASE)
            velocity_ = kZero;
        break;
    case GLFW_KEY_D:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ += kEx;
        else if(action == GLFW_RELEASE)
            velocity_ = kZero;
        break;
    case GLFW_KEY_A:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ -= kEx;
        else if(action == GLFW_RELEASE)
            velocity_ = kZero;
        break;
    case GLFW_KEY_E:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ += kEy;
        else if(action == GLFW_RELEASE)
            velocity_ = kZero;
        break;
    case GLFW_KEY_Q:
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ -= kEy;
        else if(action == GLFW_RELEASE)
            velocity_ = kZero;
        break;
    default:
        break;
    }
}

void SimpleCamera::OnMouseMove(double xpos, double ypos)
{
    if(tracking_state_ == TrackingState::Starting)
    {
        last_mouse_pos_ = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        tracking_state_ = TrackingState::Tracking;
    }
    else if(tracking_state_ == TrackingState::Tracking)
    {
        auto mouse_pos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        auto offset = mouse_pos - last_mouse_pos_;

        constexpr float sensitivity = 0.001f;
        angular_velocity_.x += offset.y * sensitivity;
        angular_velocity_.y += offset.x * sensitivity;

        last_mouse_pos_ = mouse_pos;
    }
}

void SimpleCamera::OnMouseButton(int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(action == GLFW_PRESS)
        {
            tracking_state_ = TrackingState::Starting;
        }
        else if(action == GLFW_RELEASE)
        {
            angular_velocity_ = kZero;
            tracking_state_ = TrackingState::Idling;
        }
    }
}

void SimpleCamera::OnMouseWheel(double xoffset, double yoffset)
{
    constexpr auto min_fov = glm::radians(45.0f);
    constexpr auto max_fov = glm::radians(90.0f);
    constexpr auto delta = glm::radians(1.0f);
    fov_ += (static_cast<float>(yoffset) > 0.0f)? delta : -delta;
    fov_ = glm::clamp(fov_, min_fov, max_fov);
}

void SimpleCamera::Update(double dt)
{
    auto avel = angular_velocity_ * static_cast<float>(dt);
    pitch(avel.x);
    yaw(avel.y);

    translate(velocity_ * static_cast<float>(dt));

    auto trans = glm::mat4_cast(orientation_);
    trans[3] = glm::vec4(position_, 1.0f);
    view() = glm::inverse(trans);

    const auto& depth_range = viewport().depth_range();
    proj() = glm::perspective(fov_, viewport().aspect_ratio(), depth_range.x, depth_range.y);
}

void SimpleCamera::LookAt(const glm::vec3& v)
{
    auto direction = v - position_;
    if(glm::length(direction) > 0.0f)
    {
        direction = glm::normalize(direction);
        const auto forward = orientation_ * kForward;
        // Shortest arc.
        orientation_ = glm::rotation(forward, direction) * orientation_;
    }
}

}   // namespace common::render