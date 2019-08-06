#include <array>
#include "sdf_text.h"

static const std::string vs_source =
"#version 430\n"
"layout(location = 0) in vec2 vsPosition;\n"
"layout(location = 1) in vec3 vsTexCoord;\n"
"out gl_PerVertex\n"
"{\n"
    "vec4 gl_Position;\n"
"};\n"
"out vec3 fsTexCoord;\n"
"uniform mat4 mvp;\n"
"void main(void)\n"
"{\n"
    "gl_Position = mvp * vec4(vsPosition.xy, 0.0, 1.0);\n"
    "fsTexCoord = vsTexCoord.xyz;\n"
"}\n";

#if 0
// Distance field rendering.
// 0 <= smoothness <= 1
static const std::string fs_source =
"#version 430\n"
"#extension GL_EXT_texture_array : enable\n"
"in vec3 fsTexCoord;\n"
"out vec4 outColor;\n"
"uniform sampler2DArray texture;\n"
"uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"uniform float smoothness = 0.5;\n"
"const float SMOOTHING_BASE = 0.5;\n"
"void main(void)\n"
"{\n"
    "float distance = texture2DArray(texture, fsTexCoord.xyz).r;\n"
    "if(smoothness > 0.0){\n"
        "float value = clamp(smoothness * SMOOTHING_BASE, 0.0, SMOOTHING_BASE);\n"
        "outColor.a = color.a * smoothstep(SMOOTHING_BASE - value, SMOOTHING_BASE + value, distance);\n"
    "}\n"
    "else{\n"
        "outColor.a = (distance < SMOOTHING_BASE)? 0.0 : (distance > SMOOTHING_BASE)? 1.0 : 0.5;\n"
    "}\n"
    "outColor.rgb = color.rgb;\n"
"}\n";
#else
// Distance field rendering with outline.
// 0 < smoothing < 1/3
static const std::string fs_source =
"#version 430\n"
"#extension GL_EXT_texture_array : enable\n"
"in vec3 fsTexCoord;\n"
"out vec4 outColor;\n"
"uniform sampler2DArray texture;\n"
"uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"uniform float smoothness = 0.5;\n"
"const float SMOOTHING_BASE = 0.3;\n"
"uniform vec4 outline_color = vec4(0.0, 0.0, 0.0, 1.0);\n"
"const float OUTLINE_MIN = 0.25;\n"
"const float OUTLINE_MAX = 0.50;\n"
"void main(void)\n"
"{\n"
    "vec4 baseColor = color;\n"
    "float distance = texture2DArray(texture, fsTexCoord.xyz).r;\n"
    "if(distance <= OUTLINE_MIN){\n"
        "baseColor = outline_color;\n"
    "}\n"
    "else if(distance < OUTLINE_MAX){\n"
        "float factor = smoothstep(OUTLINE_MIN, OUTLINE_MAX, distance);\n"
        "baseColor = mix(outline_color, color, factor);\n"
    "}\n"
    "if(smoothness > 0.0){\n"
        "float value = clamp(smoothness * SMOOTHING_BASE, 0.0, SMOOTHING_BASE);\n"
        "baseColor.a *= smoothstep(SMOOTHING_BASE - value, SMOOTHING_BASE + value, distance);\n"
    "}\n"
    "else{\n"
        "baseColor.a = (distance < SMOOTHING_BASE)? 0.0 : (distance > SMOOTHING_BASE)? 1.0 : 0.5;\n"
    "}\n"
    "outColor = baseColor;\n"
"}\n";
#endif

// SDFTextRenderer

SDFTextRenderer::SDFTextRenderer()
    : vs(vs_source, GL_VERTEX_SHADER), fs(fs_source, GL_FRAGMENT_SHADER)
{
    vertices.reserve(max_vertices);
    indices.reserve(max_indices);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * max_vertices, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(offsetof(Vertex, s)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * max_indices, nullptr, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    pipeline.Create();
    pipeline.SetShaderProgram(&vs);
    pipeline.SetShaderProgram(&fs);
    pipeline.SetUniform1i("texture", 0);
    pipeline.SetUniform1f("smoothness", 0.5f);
    const std::array<GLfloat, 4> outline_color = { 0.0f, 0.0f, 0.0f, 1.0f };
    pipeline.SetUniform4fv("outline_color", 1, outline_color.data());
}

SDFTextRenderer::~SDFTextRenderer()
{
    if(glIsSampler(sampler))
        glDeleteSamplers(1, &sampler);
    if(glIsBuffer(ibo))
        glDeleteBuffers(1, &ibo);
    if(glIsBuffer(vbo))
        glDeleteBuffers(1, &vbo);
    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);
}

void SDFTextRenderer::BeginRendering(GLuint texture)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pipeline.Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glBindSampler(0, sampler);
}

void SDFTextRenderer::EndRendering()
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glActiveTexture(GL_TEXTURE0);
    pipeline.Unbind();
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void SDFTextRenderer::Render()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices.size(), vertices.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(std::uint32_t) * indices.size(), indices.data());

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void SDFTextRenderer::SetOrthographicProjectionMatrix(const GLfloat* m)
{
    pipeline.SetUniformMatrix4fv("mvp", 1, GL_FALSE, m);
}

void SDFTextRenderer::SetColor(const GLfloat* color)
{
    pipeline.SetUniform4fv("color", 1, color);
}

void SDFTextRenderer::SetSmoothness(GLfloat smoothness)
{
    pipeline.SetUniform1f("smoothness", smoothness);
}

void SDFTextRenderer::SetOutlineColor(const GLfloat* color)
{
    pipeline.SetUniform4fv("outline_color", 1, color);
}

std::size_t SDFTextRenderer::GetMaxBufferLength()
{
    return max_buffer_length;
}

bool SDFTextRenderer::IsBufferEmpty()
{
    return vertices.empty();
}

void SDFTextRenderer::SetToBuffer(const Rect& bounds, const Rect& texcoord_bounds, std::uint16_t page)
{
    if(!vertices.empty())
        indices.emplace_back(vertices.size());// for degenerate triangle.

    indices.emplace_back(vertices.size());
    vertices.emplace_back(std::get<0>(bounds), std::get<1>(bounds), std::get<0>(texcoord_bounds), std::get<1>(texcoord_bounds), static_cast<float>(page)); // left / top

    indices.emplace_back(vertices.size());
    vertices.emplace_back(std::get<0>(bounds), std::get<3>(bounds), std::get<0>(texcoord_bounds), std::get<3>(texcoord_bounds), static_cast<float>(page)); // left / bottom

    indices.emplace_back(vertices.size());
    vertices.emplace_back(std::get<2>(bounds), std::get<1>(bounds), std::get<2>(texcoord_bounds), std::get<1>(texcoord_bounds), static_cast<float>(page)); // right / top

    indices.emplace_back(vertices.size());
    indices.emplace_back(vertices.size());   // for degenerate triangle.
    vertices.emplace_back(std::get<2>(bounds), std::get<3>(bounds), std::get<2>(texcoord_bounds), std::get<3>(texcoord_bounds), static_cast<float>(page)); // right / bottom
}

void SDFTextRenderer::ClearBuffer()
{
    vertices.clear();
    indices.clear();
}

// SDFText

SDFText::SDFText(const std::shared_ptr<text::Font>& font, const std::shared_ptr<SDFTextRenderer>& renderer)
    : text::Text(font, renderer)
{
}

void SDFText::SetSmoothness(GLfloat smoothness)
{
    static_cast<SDFTextRenderer*>(GetRenderer())->SetSmoothness(smoothness);
}

void SDFText::SetOutlineColor(const GLfloat* color)
{
    static_cast<SDFTextRenderer*>(GetRenderer())->SetOutlineColor(color);
}