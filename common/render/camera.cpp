#include <hasenpfote/math/utils.h>
#include "camera.h"

/*
    Normalized device coordinates:

        -1 <= nd.x <= +1
        -1 <= nd.y <= +1
        -1 <= nd.z <= +1

    Screen coordinates:

        origin.x <= s.x <= resolution.x
        resolution.y <= s.y <= origin.y
        -1 <= s.z <= +1

    About Z:

        World : ND
        -near   -1
        -far    +1
 */

namespace common::render
{

glm::vec3 Camera::world_to_screen(const glm::vec3& pos) const
{
    using hasenpfote::math::Remap;
    // wolrd to clip.
    const auto clip = proj_ * view_ * glm::vec4(pos, 1.0f);
    // clip to nd.
    const auto nd = glm::vec3(clip) / clip.w;
    // nd to screen.
    const auto& origin = vp_.origin();
    const auto& resolution = vp_.size();
    return glm::vec3(
        Remap(nd.x, -1.0f, 1.0f, origin.x, resolution.x),
        Remap(nd.y, -1.0f, 1.0f, resolution.y, origin.y),
        nd.z
    );
}

glm::vec3 Camera::screen_to_world(const glm::vec3& pos) const
{
    using hasenpfote::math::Remap;
    // screen to nd.
    const auto& origin = vp_.origin();
    const auto& resolution = vp_.size();
    const auto nd = glm::vec4(
        Remap(pos.x, origin.x, resolution.x, -1.0f, 1.0f),
        Remap(pos.y, origin.y, resolution.y, 1.0f, -1.0f),
        pos.z,
        1.0f
    );
    // nd to world.
    const auto homogeneous = glm::inverse(proj_ * view_) * nd;
    return glm::vec3(homogeneous) / homogeneous.w;
}

glm::vec3 Camera::screen_to_world(const glm::vec2& pos) const
{
    return screen_to_world(glm::vec3(pos, -1.0f));
}

glm::vec3 Camera::to_ray(const glm::vec2& pos) const
{
    using hasenpfote::math::Remap;
    // screen to nd.
    const auto& origin = vp_.origin();
    const auto& resolution = vp_.size();
    const auto nd = glm::vec4(
        Remap(pos.x, origin.x, resolution.x, -1.0f, 1.0f),
        Remap(pos.y, origin.y, resolution.y, 1.0f, -1.0f),
        -1.0f,
        1.0f
    );
    // nd to world.
    auto near = glm::inverse(proj_ * view_) * nd;
    near /= near.w;

    const auto ray = glm::vec3(near);

    return glm::normalize(ray);
}

}   // namespace common::render