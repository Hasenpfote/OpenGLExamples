#pragma once
#include <GLFW/glfw3.h>

namespace common::render
{

class FrameBuffer final
{
public:
    FrameBuffer(GLuint color, GLuint depth = 0, GLuint stencil = 0);
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator = (const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&&) = delete;
    FrameBuffer& operator = (FrameBuffer&&) = delete;

    void Bind();
    void Unbind();
    void UpdateAllMipmapLevels();

    GLuint GetColorTexture();
    GLuint GetDepthTexture();

private:
    GLuint  fbo;
    bool    is_active;
    GLint   prev_viewport[4];
};

}   // namespace common::render