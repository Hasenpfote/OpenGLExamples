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
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            if(mods == GLFW_MOD_SHIFT)
            {
                angular_velocity_.z -= 10.0f * kMouseSensitivity;
            }
            else
            {
                velocity_ += kEy;
            }
        }
        else if(action == GLFW_RELEASE)
        {
            if(mods == GLFW_MOD_SHIFT)
            {
                angular_velocity_.z = 0.0f;
            }
            else
            {
                velocity_ = kZero;
            }
        }
        break;
    case GLFW_KEY_Q:
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            if(mods == GLFW_MOD_SHIFT)
            {
                angular_velocity_.z += 10.0f * kMouseSensitivity;
            }
            else
            {
                velocity_ -= kEy;
            }
        }
        else if(action == GLFW_RELEASE)
        {
            if (mods == GLFW_MOD_SHIFT)
            {
                angular_velocity_.z = 0.0f;
            }
            else
            {
                velocity_ = kZero;
            }
        }
        break;
    default:
        break;
    }
}

void SimpleCamera::OnMouseMove(double xpos, double ypos)
{
    // l_drag_state_
    if(l_drag_info_.state_ == DragInfo::State::Enter)
    {
        l_drag_info_.last_pos_ = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        l_drag_info_.state_ = DragInfo::State::Move;
    }
    else if(l_drag_info_.state_ == DragInfo::State::Move)
    {
        auto mouse_pos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        auto offset = mouse_pos - l_drag_info_.last_pos_;

        angular_velocity_.x += offset.y * kMouseSensitivity;
        angular_velocity_.y += offset.x * kMouseSensitivity;

        l_drag_info_.last_pos_ = mouse_pos;
    }
    // r_drag_state_
    if(r_drag_info_.state_ == DragInfo::State::Enter)
    {
        r_drag_info_.last_pos_ = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        r_drag_info_.state_ = DragInfo::State::Move;
    }
    else if(r_drag_info_.state_ == DragInfo::State::Move)
    {
        auto mouse_pos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        auto offset = mouse_pos - r_drag_info_.last_pos_;

        angular_velocity_.z += offset.x * kMouseSensitivity;

        r_drag_info_.last_pos_ = mouse_pos;
    }
}

void SimpleCamera::OnMouseButton(int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        if(action == GLFW_PRESS)
        {
            l_drag_info_.state_ = DragInfo::State::Enter;
        }
        else if(action == GLFW_RELEASE)
        {
            angular_velocity_.x = 0.0f;
            angular_velocity_.y = 0.0f;
            l_drag_info_.state_ = DragInfo::State::Leave;
        }
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        if(action == GLFW_PRESS)
        {
            r_drag_info_.state_ = DragInfo::State::Enter;
        }
        else if(action == GLFW_RELEASE)
        {
            angular_velocity_.z = 0.0f;
            r_drag_info_.state_ = DragInfo::State::Leave;
        }
        break;
    default:
        break;
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
    rotate(angular_velocity_ * static_cast<float>(dt));

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