#pragma once
#include "../shader/shader.h"
#include "text.h"

namespace common::render::text
{

/*!
 * @class SDFTextRenderer
 * @brief Signed distance field font rendering.
 */
class SDFTextRenderer final : public ITextRenderer
{
    friend class SDFText;

    static constexpr std::size_t max_buffer_length = 32;
    static constexpr GLsizei max_vertices = max_buffer_length * 4;
    static constexpr GLsizei max_indices = max_vertices + max_buffer_length * 2 - 1;

public:
    SDFTextRenderer();
    ~SDFTextRenderer();

private:
    void BeginRendering(GLuint texture) override;
    void EndRendering() override;
    void Render() override;

    void SetOrthographicProjectionMatrix(const GLfloat* m) override;
    void SetColor(const GLfloat* color) override;

    std::size_t GetMaxBufferLength() override;
    bool IsBufferEmpty() override;
    void SetToBuffer(const Rect& bounds, const Rect& texcoord_bounds, std::uint16_t page) override;
    void ClearBuffer() override;

    void SetSmoothness(GLfloat smoothness);
    void SetOutlineColor(const GLfloat* color);

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
        GLfloat layer;
        Vertex(GLfloat x, GLfloat y, GLfloat s, GLfloat t, GLfloat layer)
            : x(x), y(y), s(s), t(t), layer(layer)
        {
        }
    };

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    std::unique_ptr<shader::Program> vs;
    std::unique_ptr<shader::Program> fs;
    std::unique_ptr<shader::ProgramPipeline> pipeline;
};

/*!
 * @class SDFText
 * @brief Signed distance field font.
 */
class SDFText final : public Text
{
public:
    SDFText(const std::shared_ptr<Font>& font, const std::shared_ptr<SDFTextRenderer>& renderer);
    ~SDFText() = default;

    void SetSmoothness(GLfloat smoothness);
    void SetOutlineColor(const GLfloat* color);
};

}   // namespace common::render::text