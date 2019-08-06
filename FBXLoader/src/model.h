#pragma once
#include <string>
#include <GL/glew.h>
#include "../../common/system.h"
#include "fbx_loader.h"

class Material final
{
public:
    Material();
    ~Material();
    void Setup(const fbxloader::Material& material);

    std::string GetDiffuseTextureName() const { return diffuse_texture_name; }
    GLuint GetDiffuseSampler() const { return diffuse_sampler; }
    bool IsAlphaEnabled() const { return is_alpha_enabled; }
    bool IsDoubleSideEnabled() const { return is_double_side_enabled; }

private:
    std::string diffuse_texture_name;
    GLuint diffuse_sampler = 0;
    bool is_alpha_enabled = false;
    bool is_double_side_enabled = false;

};

class Mesh final
{
public:
    Mesh();
    ~Mesh();

    void Setup(const fbxloader::Mesh& mesh);
    void Draw();

    std::string GetNodeName() const { return node_name; }
    std::string GetMaterialName() const { return material_name; }
    const std::vector<std::string>& GetJointNames() const { return joint_names; }
    bool IsSkinnedMesh() const { return is_skinned_mesh; }

private:
    static constexpr auto position_index = 0;
    static constexpr auto normal_index = 1;
    static constexpr auto texcoord0_index = 2;
    static constexpr auto joint_indices_index = 3;
    static constexpr auto joint_weights_index = 4;

private:
    std::string node_name;
    std::string material_name;
    std::vector<std::string> joint_names;

    GLuint vao = 0;
    GLuint position_buffer_object = 0;
    GLuint normal_buffer_object = 0;
    GLuint texcoord0_buffer_object = 0;
    GLuint joint_indices_buffer_object = 0;
    GLuint joint_weights_buffer_object = 0;
    GLuint index_buffer_object = 0;
    GLsizei count = 0;

    bool is_skinned_mesh = false;

public:
    std::string texture_name;
};

class Model final
{
    using System = common::System;
    using Texture = common::render::Texture;
    using ShaderProgram = common::render::ShaderProgram;
    using ShaderPipeline = common::render::ShaderPipeline;

public:
    Model();
    ~Model();

    void Setup(fbxloader::Model& model, GLuint common_matrices);
    void DrawOpaqueMeshes(const hasenpfote::math::CMatrix4& model);
    void DrawTransparentMeshes(const hasenpfote::math::CMatrix4& model);
    void DrawSkeleton();
    void Update(double dt);

private:
    std::unordered_map<std::string, std::unique_ptr<Material>> material;
    std::vector<std::unique_ptr<Mesh>> opaque_meshes;
    std::vector<std::unique_ptr<Mesh>> transparent_meshes;
    std::unique_ptr<Skeleton> skeleton;

    std::array<hasenpfote::math::CMatrix4, 128> palette;

    GLuint pipeline;
    GLuint common_matrices;

    ShaderProgram* vs;
    ShaderProgram* fs;
};