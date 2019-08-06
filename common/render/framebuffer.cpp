#include <cassert>
#include <memory>
#include <GL/glew.h>
#include "../logger.h"
#include "framebuffer.h"

namespace
{

void GetFrameBufferSize(GLuint fbo, GLint* width, GLint* height)
{
    assert(glIsFramebuffer(fbo));

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLint texture;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &texture);
    if(glIsTexture(texture)){

        GLint prev_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, height);
        glBindTexture(GL_TEXTURE_2D, prev_texture);
    }
    else{
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &texture);
        if(glIsTexture(texture)){
            GLint prev_texture;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, width);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, height);
            glBindTexture(GL_TEXTURE_2D, prev_texture);
        }
        else{
            assert(false);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}

namespace common::render
{

FrameBuffer::FrameBuffer(GLuint color, GLuint depth, GLuint stencil)
{
    LOG_I("Creating FBO.");

    memset(prev_viewport, 0, sizeof(prev_viewport));
    is_active = false;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if(glIsTexture(color))
    {
        LOG_I("Attaching color texture to fbo. [id=" << color << "]");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

        GLint encoding = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &encoding);
        if(encoding == GL_LINEAR)
            LOG_I("Framebuffer attachment color encoding is linear.");
        else if(encoding == GL_SRGB)
            LOG_I("Framebuffer attachment color encoding is sRGB.");
        else
            assert(false);
    }

    if(glIsTexture(depth))
    {
        LOG_I("Attaching depth texture to fbo. [id=" << depth << "]");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    }

    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_E("framebuffer is not complete: " << status);
        assert(false);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    LOG_I("FBO created successfully. [id=" << fbo << "]");
}

FrameBuffer::~FrameBuffer()
{
    if(glIsFramebuffer(fbo))
        glDeleteFramebuffers(1, &fbo);
}

void FrameBuffer::Bind()
{
    assert(!is_active);

    is_active = true;
    glGetIntegerv(GL_VIEWPORT, prev_viewport);

    GLint width, height;
    GetFrameBufferSize(fbo, &width, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

void FrameBuffer::Unbind()
{
    assert(is_active);

    is_active = false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
}

void FrameBuffer::UpdateAllMipmapLevels()
{
    auto texture = GetColorTexture();
    if(!glIsTexture(texture))
        return;

    GLint prev_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    GLint max_level;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &max_level);
    if(max_level > 0)
        glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, prev_texture);
}

GLuint FrameBuffer::GetColorTexture()
{
    GLint texture = 0;
    GLint prev_fbo;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &texture);
    glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);

    return texture;
}

GLuint FrameBuffer::GetDepthTexture()
{
    GLint texture = 0;
    GLint prev_fbo;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &texture);
    glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);

    return texture;
}

}   // namespace common::render