#pragma once
#include <GLFW/glfw3.h>

class FullscreenPassGeometry final
{
public:
    FullscreenPassGeometry();
    ~FullscreenPassGeometry();

    void Draw();

private:
    GLuint vao;
    GLuint position_buffer_object;
    GLuint index_buffer_object;
    GLsizei index_count;
};