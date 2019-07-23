#if 0
#include "simple_text_renderer.h"

static const std::string vs_source =
"#version 430\n"
"layout(location = 0) in vec4 vsCoord;\n"
"out gl_PerVertex\n"
"{\n"
    "vec4 gl_Position;\n"
"};\n"
"out vec2 fsTCoord;\n"
"uniform mat4 mvp;\n"
"void main(void)\n"
"{\n"
    "gl_Position = mvp * vec4(vsCoord.xy, 0.0, 1.0);\n"
    "fsTCoord = vsCoord.zw;\n"
"}\n";

#if 0
// Distance field rendering.
// 0 <= smoothness <= 1
static const std::string fs_source =
"#version 430\n"
"in vec2 fsTCoord;\n"
"out vec4 OutColor;\n"
"uniform sampler2D texture;\n"
"uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"uniform float smoothness = 0.5;\n"
"const float SMOOTHING_BASE = 0.5;\n"
"void main(void)\n"
"{\n"
    "float distance = texture2D(texture, fsTCoord).r;\n"
    "if(smoothness > 0.0){\n"
        "float value = clamp(smoothness * SMOOTHING_BASE, 0.0, SMOOTHING_BASE);\n"
        "OutColor.a = color.a * smoothstep(SMOOTHING_BASE - value, SMOOTHING_BASE + value, distance);\n"
    "}\n"
    "else{\n"
        "OutColor.a = (distance < SMOOTHING_BASE)? 0.0 : (distance > SMOOTHING_BASE)? 1.0 : 0.5;\n"
    "}\n"
    "OutColor.rgb = color.rgb;\n"
"}\n";
#else
// Distance field rendering with outline.
// 0 < smoothing < 1/3
static const std::string fs_source =
"#version 430\n"
"in vec2 fsTCoord;\n"
"out vec4 OutColor;\n"
"uniform sampler2D texture;\n"
"uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"uniform float smoothness = 0.5;\n"
"const float SMOOTHING_BASE = 0.3;\n"
"const vec4 OUTLINE_COLOR = vec4(1.0, 0.0, 0.0, 1.0);\n"
"const float OUTLINE_MIN = 0.45;\n"
"const float OUTLINE_MAX = 0.50;\n"
"void main(void)\n"
"{\n"
    "vec4 baseColor = color;\n"
    "float distance = texture2D(texture, fsTCoord).r;\n"
    "if(distance <= OUTLINE_MIN){\n"
        "baseColor = OUTLINE_COLOR;\n"
    "}\n"
    "else if(distance < OUTLINE_MAX){\n"
        "float factor = smoothstep(OUTLINE_MIN, OUTLINE_MAX, distance);\n"
        "baseColor = mix(OUTLINE_COLOR, color, factor);\n"
    "}\n"
    "if(smoothness > 0.0){\n"
        "float value = clamp(smoothness * SMOOTHING_BASE, 0.0, SMOOTHING_BASE);\n"
        "baseColor.a *= smoothstep(SMOOTHING_BASE - value, SMOOTHING_BASE + value, distance);\n"
    "}\n"
    "else{\n"
        "baseColor.a = (distance < SMOOTHING_BASE)? 0.0 : (distance > SMOOTHING_BASE)? 1.0 : 0.5;\n"
    "}\n"
    "OutColor = baseColor;\n"
"}\n";
#endif

SimpleTextRenderer::SimpleTextRenderer()
{
    vertices.reserve(max_vertices);
    indices.reserve(max_indices);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * max_vertices, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * max_indices, nullptr, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    vs.Create(GL_VERTEX_SHADER, vs_source);
    fs.Create(GL_FRAGMENT_SHADER, fs_source);
    pipeline.Create();
    pipeline.SetShaderProgram(&vs);
    pipeline.SetShaderProgram(&fs);
    pipeline.SetUniform1i("texture", 0);
    pipeline.SetUniform1f("smoothness", 0.5f);
}

SimpleTextRenderer::~SimpleTextRenderer()
{
    if(glIsBuffer(ibo))
        glDeleteBuffers(1, &ibo);
    if(glIsBuffer(vbo))
        glDeleteBuffers(1, &vbo);
    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);
    if(glIsSampler(sampler))
        glDeleteSamplers(1, &sampler);
}

void SimpleTextRenderer::BeginRendering(GLuint texture)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pipeline.Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindSampler(0, sampler);
}

void SimpleTextRenderer::EndRendering()
{
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    pipeline.Unbind();
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void SimpleTextRenderer::Render()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices.size(), vertices.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(std::uint32_t) * indices.size(), indices.data());

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, nullptr);
}

void SimpleTextRenderer::SetOrthographicProjectionMatrix(const GLfloat* m)
{
    pipeline.SetUniformMatrix4fv("mvp", 1, GL_FALSE, m);
}

void SimpleTextRenderer::SetColor(const GLfloat* color)
{
    pipeline.SetUniform4fv("color", 1, color);
}

void SimpleTextRenderer::SetSmoothness(GLfloat smoothness)
{
    pipeline.SetUniform1f("smoothness", smoothness);
}

std::size_t SimpleTextRenderer::GetMaxBufferLength()
{
    return max_buffer_length;
}

bool SimpleTextRenderer::IsBufferEmpty()
{
    return vertices.empty();
}

void SimpleTextRenderer::SetToBuffer(const Rect& pos, const Rect& texcoord)
{
    if(!vertices.empty())
        indices.emplace_back(vertices.size());// for degenerate triangle.

    indices.emplace_back(vertices.size());
    vertices.emplace_back(std::get<0>(pos), std::get<1>(pos), std::get<0>(texcoord), std::get<1>(texcoord)); // left / top

    indices.emplace_back(vertices.size());
    vertices.emplace_back(std::get<0>(pos), std::get<3>(pos), std::get<0>(texcoord), std::get<3>(texcoord)); // left / bottom

    indices.emplace_back(vertices.size());
    vertices.emplace_back(std::get<2>(pos), std::get<1>(pos), std::get<2>(texcoord), std::get<1>(texcoord)); // right / top

    indices.emplace_back(vertices.size());
    indices.emplace_back(vertices.size());   // for degenerate triangle.
    vertices.emplace_back(std::get<2>(pos), std::get<3>(pos), std::get<2>(texcoord), std::get<3>(texcoord)); // right / bottom
}

void SimpleTextRenderer::ClearBuffer()
{
    vertices.clear();
    indices.clear();
}
#endif