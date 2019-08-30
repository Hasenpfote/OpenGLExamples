#pragma once
#include <glm/glm.hpp>

namespace common::render
{

class Viewport
{
public:
    Viewport() = default;
    virtual ~Viewport() = default;
    Viewport(const Viewport&) = default;
    Viewport& operator = (const Viewport&) = default;
    Viewport(Viewport&&) = default;
    Viewport& operator = (Viewport&&) = default;

    Viewport(float x, float y, float width, float height, float min_depath, float max_depath)
        : origin_(x, y), size_(width, height), depth_range_(min_depath, max_depath)
    {}

    Viewport(const glm::vec2& origin, const glm::vec2& size, const glm::vec2& depth_range)
        : origin_(origin), size_(size), depth_range_(depth_range)
    {}

    const glm::vec2& origin() const noexcept { return origin_; }
    glm::vec2& origin() noexcept { return origin_; }

    const glm::vec2& size() const noexcept { return size_; }
    glm::vec2& size() noexcept { return size_; }

    const glm::vec2& depth_range() const noexcept { return depth_range_; }
    glm::vec2& depth_range() noexcept { return depth_range_; }

    float aspect_ratio() const noexcept { return size_.x / size_.y; }

private:
    glm::vec2 origin_;
    glm::vec2 size_;
    glm::vec2 depth_range_;
};

class Camera
{
public:
    Camera() = default;
    virtual ~Camera() = default;
    Camera(const Camera&) = default;
    Camera& operator = (const Camera&) = default;
    Camera(Camera&&) = default;
    Camera& operator = (Camera&&) = default;

    const glm::mat4& view() const noexcept { return view_; }
    glm::mat4& view() noexcept { return view_; }

    const glm::mat4& proj() const noexcept { return proj_; }
    glm::mat4& proj() noexcept { return proj_; }

    const Viewport& viewport() const noexcept { return vp_; }
    Viewport& viewport() noexcept { return vp_; }

    /*!
     * Convert world to screen coords.
     * @param[in] world coords.
     * @return screen coords.
     */
    glm::vec3 world_to_screen(const glm::vec3& pos) const;

    /*!
     * Convert screen to world coords.
     * @param[in] screen coords.
     * @return world coords.
     */
    glm::vec3 screen_to_world(const glm::vec3& pos) const;
    glm::vec3 screen_to_world(const glm::vec2& pos) const;

    /*!
     * Convert from screen position to ray vector.
     * @param[in] screen coords.
     * @return world coords.
     */
    glm::vec3 to_ray(const glm::vec2& pos) const;

private:
    glm::mat4 view_;
    glm::mat4 proj_;
    Viewport vp_;
};

}   // namespace common::render