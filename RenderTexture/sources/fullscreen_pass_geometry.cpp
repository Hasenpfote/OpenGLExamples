#include <cassert>
#include <GL/glew.h>
#include "../../Common/system.h"
#include "fullscreen_pass_geometry.h"

FullscreenPassGeometry::FullscreenPassGeometry()
{
}

FullscreenPassGeometry::~FullscreenPassGeometry()
{
    if(glIsBuffer(position_buffer_object))
        glDeleteBuffers(1, &position_buffer_object);
    if(glIsBuffer(index_buffer_object))
        glDeleteBuffers(1, &index_buffer_object);
    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);
}

void FullscreenPassGeometry::Initialize()
{
    static float vertices[] =
    {
        -1.0f, -1.0f, // left bottom
         1.0f, -1.0f, // right bottom  
         1.0f,  1.0f, // right top
        -1.0f,  1.0f  // left top
    };

    static int indices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    index_count = sizeof(indices) / sizeof(indices[0]);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &position_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &index_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void FullscreenPassGeometry::Draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}