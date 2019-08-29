#include <GL/glew.h>
#include "fullscreen_quad.h"

namespace common::render
{

FullScreenQuad::FullScreenQuad()
{
#if defined(COMMON_RENDER_FULLSCREEN_QUAD_USE_SUPER_TRIANGLE)
    const GLfloat vertices[] =
    {
        -1.0f, -1.0f, // left bottom
         3.0f, -1.0f, // right bottom
        -1.0f,  3.0f  // left top
    };
#else
    const GLfloat vertices[] =
    {
        -1.0f, -1.0f, // left bottom
         1.0f, -1.0f, // right bottom
         1.0f,  1.0f, // right top
        -1.0f,  1.0f  // left top
    };
    const GLint indices[] =
    {
        0, 1, 2,
        0, 2, 3
    };
    num_indices_ = sizeof(indices) / sizeof(indices[0]);
#endif

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &pbo_);
    glBindBuffer(GL_ARRAY_BUFFER, pbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

#if !defined(COMMON_RENDER_FULLSCREEN_QUAD_USE_SUPER_TRIANGLE)
    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
#endif
    glBindVertexArray(0);
}

FullScreenQuad::~FullScreenQuad()
{
#if !defined(COMMON_RENDER_FULLSCREEN_QUAD_USE_SUPER_TRIANGLE)
    if(glIsBuffer(ibo_))
        glDeleteBuffers(1, &ibo_);
#endif
    if(glIsBuffer(pbo_))
        glDeleteBuffers(1, &pbo_);
    if(glIsVertexArray(vao_))
        glDeleteVertexArrays(1, &vao_);
}

void FullScreenQuad::Draw()
{
    glBindVertexArray(vao_);
#if defined(COMMON_RENDER_FULLSCREEN_QUAD_USE_SUPER_TRIANGLE)
    glDrawArrays(GL_TRIANGLES, 0, 3);
#else
    glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_INT, nullptr);
#endif
    glBindVertexArray(0);
}

}   // namespace common::render