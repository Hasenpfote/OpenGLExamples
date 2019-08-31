#include <iostream>
#include <GLFW/glfw3.h>
#include "../logger.h"
#include "simple_camera.h"

namespace
{

glm::quat shortest_arc(const glm::vec3& a, const glm::vec3& b)
{
    const auto d = glm::dot(a, b);
    const auto s = glm::sqrt((1.0f + d) * 2.0f);
    const auto c = glm::cross(a, b);

    return glm::quat(s * 0.5f, c / s);
}

}   // namespace

namespace common::render
{

void SimpleCamera::OnKey(int key, int scancode, int action, int mods)
{
    switch(key)
    {
    case GLFW_KEY_W:
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ += glm::vec3(0.0f, 0.0f, -1.0f);
        else if(action == GLFW_RELEASE)
            velocity_ = glm::vec3(0.0f);
        break;
    case GLFW_KEY_S:
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ -= glm::vec3(0.0f, 0.0f, -1.0f);
        else if (action == GLFW_RELEASE)
            velocity_ = glm::vec3(0.0f);
        break;
    case GLFW_KEY_D:
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ += glm::vec3(1.0f, 0.0f, 0.0f);
        else if(action == GLFW_RELEASE)
            velocity_ = glm::vec3(0.0f);
        break;
    case GLFW_KEY_A:
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ -= glm::vec3(1.0f, 0.0f, 0.0f);
        else if(action == GLFW_RELEASE)
            velocity_ = glm::vec3(0.0f);
        break;
    case GLFW_KEY_E:
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ += glm::vec3(0.0f, 1.0f, 0.0f);
        else if(action == GLFW_RELEASE)
            velocity_ = glm::vec3(0.0f);
        break;
    case GLFW_KEY_Q:
        if(action == GLFW_PRESS || action == GLFW_REPEAT)
            velocity_ -= glm::vec3(0.0f, 1.0f, 0.0f);
        else if(action == GLFW_RELEASE)
            velocity_ = glm::vec3(0.0f);
        break;
    default:
        break;
    }
}

void SimpleCamera::OnMouseMove(double xpos, double ypos)
{
    static glm::vec2 prev(0.0f);

    if(tracking_state_ == TrackingState::Starting)
    {
        prev = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        tracking_state_ = TrackingState::Tracking;
    }
    else if(tracking_state_ == TrackingState::Tracking)
    {
        auto current = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        auto offset = current - prev;

        constexpr float sensitivity = 0.001f;
        angular_velocity_.x += offset.y * sensitivity;
        angular_velocity_.y += offset.x * sensitivity;

        prev = current;
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
            angular_velocity_ = glm::vec3(0.0f);
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

    auto trans = glm::translate(glm::mat4(1.0f), position_) * glm::mat4_cast(orientation_);
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
        //
        const auto forward = orientation_ * glm::vec3(0.0f, 0.0f, -1.0f);
        if(glm::dot(direction, forward) < 1.0f)
        {
            orientation_ = shortest_arc(direction, forward);
        }
    }
}

}   // namespace common::render