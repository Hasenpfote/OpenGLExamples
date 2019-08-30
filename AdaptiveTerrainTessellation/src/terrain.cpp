#include <cassert>
#include <vector>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include "terrain.h"

Terrain::Terrain()
{
    lod_factor = 10.0f;
    horizontal_scale = 1000.0f;
    vertical_scale = 300.0f;
    draw_mode = DrawMode::Solid;
    light_direction = glm::normalize(glm::vec3(1.0f, 1.0f,-1.0));
}

Terrain::~Terrain()
{
    if(glIsBuffer(texcoord_buffer_object))
        glDeleteBuffers(1, &texcoord_buffer_object);
    if(glIsBuffer(index_buffer_object))
        glDeleteBuffers(1, &index_buffer_object);
    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);
    if(glIsSampler(sampler))
        glDeleteSamplers(1, &sampler);
}

void Terrain::Initialize()
{
    /*
        tess contorol shader の outer と合わせる
                         e3
        +--- x       v3 --- v2
        |         e0 |       | e2
        z            v0 --- v1
                         e1
     */
    constexpr int size = 10+1;// 50 + 1;

    std::vector<float> vertices;
    std::vector<float> coords;
    for(int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            const float u = static_cast<float>(j);
            const float v = static_cast<float>(i);
            coords.push_back(u/(size-1));
            coords.push_back(v/(size-1));
        }
    }

    std::vector<int> indices;
    for(int i = 0; i < size - 1; i++){
        for(int j = 0; j < size - 1; j++){
            indices.push_back((i+1)*size+j);
            indices.push_back((i+1)*size+j+1);
            indices.push_back(i*size+j+1);
            indices.push_back(i*size+j);
        }
    }
    num_indices = indices.size();
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &texcoord_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * coords.size(), coords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &index_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    auto& rm = System::GetMutableInstance().GetResourceManager();

    diffuse_map = rm.GetResource<Texture>("assets/textures/terrain.png")->GetTexture();
    height_map = rm.GetResource<Texture>("assets/textures/heightmap_linear.png")->GetTexture();

    // for solid model
    pipeline1 = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/terrain.vs"),
            rm.GetResource<Program>("assets/shaders/terrain.tcs"),
            rm.GetResource<Program>("assets/shaders/terrain.tes"),
            rm.GetResource<Program>("assets/shaders/terrain.gs"),
            rm.GetResource<Program>("assets/shaders/terrain.fs")})
            );

    // for wireframe model
    pipeline2 = std::make_unique<ProgramPipeline>(
        ProgramPipeline::ProgramPtrSet({
            rm.GetResource<Program>("assets/shaders/terrain.vs"),
            rm.GetResource<Program>("assets/shaders/terrain.tcs"),
            rm.GetResource<Program>("assets/shaders/terrain.tes"),
            rm.GetResource<Program>("assets/shaders/terrain_wf.gs"),
            rm.GetResource<Program>("assets/shaders/terrain_wf.fs") })
            );
}

void Terrain::Draw()
{
    if(draw_mode == DrawMode::Solid){
        DrawSolid();
    }
    else{
        DrawWireFrame();
    }
}

void Terrain::DrawSolid()
{
    auto& uniform = pipeline1->GetPipelineUniform();
    uniform.Set("diffuse_map", 0);
    uniform.Set("height_map", 1);
    uniform.Set("horizontal_scale", horizontal_scale);
    uniform.Set("vertical_scale", vertical_scale);
    uniform.Set("lod_factor", lod_factor);

    auto& camera = System::GetMutableInstance().GetCamera();
    const auto& resolution = camera.viewport().size();
    auto mvp = camera.proj() * camera.view();
    uniform.Set("mvp", &mvp, 1, false);
    uniform.Set("vp_size", glm::vec2(resolution.x, resolution.y));

    uniform.Set("light_direction", light_direction);

    pipeline1->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_map);
        glBindSampler(0, sampler);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, height_map);
        glBindSampler(1, sampler);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glBindVertexArray(vao);
        glDrawElements(GL_PATCHES, num_indices, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
    pipeline1->Unbind();
}

void Terrain::DrawWireFrame()
{
    auto& uniform = pipeline2->GetPipelineUniform();
    uniform.Set("height_map", 0);
    uniform.Set("horizontal_scale", horizontal_scale);
    uniform.Set("vertical_scale", vertical_scale);
    uniform.Set("lod_factor", lod_factor);

    auto& camera = System::GetMutableInstance().GetCamera();
    const auto& resolution = camera.viewport().size();
    auto mvp = camera.proj() * camera.view();
    uniform.Set("mvp", &mvp, 1, false);
    uniform.Set("vp_size", glm::vec2(resolution.x, resolution.y));

    pipeline2->Bind();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, height_map);
        glBindSampler(0, sampler);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glBindVertexArray(vao);
        glDrawElements(GL_PATCHES, num_indices, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
    pipeline2->Unbind();
}