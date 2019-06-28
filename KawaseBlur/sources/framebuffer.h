#pragma once
#include <GLFW/glfw3.h>

class FrameBuffer final
{
public:
    FrameBuffer(GLuint color, GLuint depth, GLuint stencil);
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator = (const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&&) = delete;
    FrameBuffer& operator = (FrameBuffer&&) = delete;

    void Bind();
    void Unbind();

    GLuint GetColorTexture();
    GLuint GetDepthTexture();

private:
    GLuint  fbo;
    bool    is_active;
    GLint   prev_viewport[4];
};