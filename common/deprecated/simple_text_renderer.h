#pragma once
#if 0
#include "shader.h"
#include "text.h"

class SimpleTextRenderer final : public text::IRenderer
{
    static constexpr std::size_t max_buffer_length = 32;
    static constexpr GLsizei max_vertices = max_buffer_length * 4;
    static constexpr GLsizei max_indices = max_vertices + max_buffer_length * 2 - 1;

public:
    SimpleTextRenderer();
    ~SimpleTextRenderer();

private:
    void BeginRendering(GLuint texture) override;
    void EndRendering() override;
    void Render() override;

    void SetOrthographicProjectionMatrix(const GLfloat* m) override;
    void SetColor(const GLfloat* color) override;

    std::size_t GetMaxBufferLength() override;
    bool IsBufferEmpty() override;
    void SetToBuffer(const Rect& pos, const Rect& texcoord) override;
    void ClearBuffer() override;

public:
    void SetSmoothness(GLfloat smoothness);

private:
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint sampler;
    struct Vertex
    {
        GLfloat x;
        GLfloat y;
        GLfloat s;
        GLfloat t;
        Vertex(GLfloat x, GLfloat y, GLfloat s, GLfloat t)
            : x(x), y(y), s(s), t(t)
        {
        }
    };
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    ShaderProgram vs;
    ShaderProgram fs;
    ShaderPipeline pipeline;
};
#endif