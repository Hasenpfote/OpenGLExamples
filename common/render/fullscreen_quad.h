#pragma once
#include <GLFW/glfw3.h>

#define COMMON_RENDER_FULLSCREEN_QUAD_USE_SUPER_TRIANGLE

namespace common::render
{

class FullScreenQuad final
{
public:
    FullScreenQuad();
    ~FullScreenQuad();

    FullScreenQuad(const FullScreenQuad&) = delete;
    FullScreenQuad& operator = (const FullScreenQuad&) = delete;

    FullScreenQuad(FullScreenQuad&&) = default;
    FullScreenQuad& operator = (FullScreenQuad&&) = default;

    void Draw();

private:
    GLuint vao_;
    GLuint pbo_;
#if !defined(COMMON_RENDER_FULLSCREEN_QUAD_USE_SUPER_TRIANGLE)
    GLuint ibo_;
    GLsizei num_indices_;
#endif
};

}   // namespace common::render