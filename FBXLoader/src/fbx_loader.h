#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <array>
#include <fbxsdk.h>
#include "skeleton.h"

namespace fbxloader{

// 検証用にメンバ変数へのアクセス権限は public
/*
namespace fbxsdk{
class FbxMesh;
class FbxSurfaceMaterial;
}
*/
//constexpr int max_infulences_per_vertex = 4;
using Weight = std::tuple<int, double>;

class Vertex final
{
public:
    FbxDouble3 position;
    FbxDouble3 normal;
    FbxDouble2 texcoord0;
    std::array<Weight, 4> weights;

    bool operator == (const Vertex& v) const {
        return std::memcmp(this, &v, sizeof(Vertex)) == 0;
    }
};

class Texture final
{
public:
    std::string filename;
    std::string uvset;

public:
    Texture() = default;
    ~Texture() = default;
};

class Material final
{
public:
    enum class Type
    {
        Lambert,
        Phong,
        Shader
    };

    class Lambert
    {
    public:
        std::array<float, 3> ambient;
        std::array<float, 3> diffuse;
        std::array<float, 3> emissive;
        float opacity;
    };

    class Phong
    {
    public:
        std::array<float, 3> ambient;
        std::array<float, 3> diffuse;
        std::array<float, 3> emissive;
        std::array<float, 3> specular;
        float opacity;
        float shininess;
        float reflectivity;
    };

    class Shader
    {
    public:
        std::string filename;
        std::unique_ptr<Texture> diffuse;
        std::unique_ptr<Texture> normal;
        std::unique_ptr<Texture> specular;
        std::unique_ptr<Texture> falloff;
        std::unique_ptr<Texture> reflectionmap;
    };

    std::unique_ptr<Lambert> lambert;
    std::unique_ptr<Phong> phong;
    std::unique_ptr<Shader> shader;
    bool is_double_side_enabled = false;
    bool is_alpha_enabled = false;

public:
    Material() = default;
    ~Material() = default;
};

class Mesh final
{
public:
    std::string node_name;
    std::string material_name;
    std::vector<int> indices;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords0;
    std::vector<std::uint8_t> joint_indices;
    std::vector<float> joint_weights;
    std::vector<std::string> joint_names;

    bool is_skinned_mesh = false;

public:
    Mesh() {}
    ~Mesh() {}
};

class Model final
{
public:
    std::vector<std::unique_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::unique_ptr<Material>> material;

    std::unique_ptr<Skeleton> skeleton;

public:
    Model();
    ~Model();

    bool Load(const std::string& filepath);

private:
    void ParseSkelton(const fbxsdk::FbxNode* node);
    void ParseSkelton(const fbxsdk::FbxNode* node, int depth, int index, int parent);

    std::unique_ptr<Mesh> ParseMesh(const fbxsdk::FbxMesh*);
    void OptimizeMesh(
        const std::vector<FbxDouble3>& positions,
        const std::vector<FbxDouble3>& normals,
        const std::vector<FbxDouble2>& texcoords,
        const std::vector<std::array<Weight, 4>>& joint_weights,
        std::vector<Vertex>& vertices,
        std::vector<int>& indices);

    std::vector<FbxDouble3> GetControlPointsByPolygonVertex(const fbxsdk::FbxMesh* mesh);
    std::vector<FbxDouble3> GetNormalsByPolygonVertex(const fbxsdk::FbxMesh* mesh);
    std::vector<FbxDouble2> GetUVsByPolygonVertex(const fbxsdk::FbxMesh* mesh, int index);
    std::unordered_map<std::string, std::vector<FbxDouble2>> GetUVsByPolygonVertex(const fbxsdk::FbxMesh* mesh);
    void GetWeightsByPolygonVertex(const fbxsdk::FbxMesh* mesh,
        std::vector<std::string>& joint_names,
        std::vector<std::array<Weight, 4>>& joint_weights);

    std::unique_ptr<Material> ParseMaterial(const fbxsdk::FbxSurfaceMaterial* material);
};

class Animation final
{
public:
    bool Load(const std::string& filepath);
};

}