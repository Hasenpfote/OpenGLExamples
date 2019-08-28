#include <cassert>
#include <iostream>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fbxsdk.h>
#include "fbx_loader.h"

namespace fbxloader{

Model::Model()
{
}

Model::~Model()
{
    std::cout << __func__ << std::endl;
}

bool Model::Load(const std::string& filepath)
{
    FbxManager* manager = FbxManager::Create();
    FbxImporter* importer = FbxImporter::Create(manager, "");
    FbxScene* scene = FbxScene::Create(manager, "");
    importer->Initialize(filepath.c_str());
    importer->Import(scene);
    {
        // 軸の設定.
        if(scene->GetGlobalSettings().GetAxisSystem() != FbxAxisSystem::OpenGL){
            FbxAxisSystem::OpenGL.ConvertScene(scene);
        }

        // 単位を統一する.
        FbxSystemUnit SceneSystemUnit = scene->GetGlobalSettings().GetSystemUnit();
        FbxSystemUnit::cm.ConvertScene(scene);

        // 三角形分割を行う.
        // 以降、すべてのポリゴンは 3 頂点を前提として処理を行う
        FbxGeometryConverter gc(manager);
        gc.Triangulate(scene, true);

        // スケルトンの取得
        const auto num_skeletons = scene->GetMemberCount<fbxsdk::FbxSkeleton>();
        if(num_skeletons > 0){
            skeleton = std::unique_ptr<Skeleton>(new Skeleton());
            ParseSkelton(scene->GetRootNode());
        }

        // メッシュの解析
        const auto num_meshes = scene->GetMemberCount<FbxMesh>();
        std::cout << "Meshes: " << num_meshes << std::endl;
        this->meshes.reserve(num_meshes);
        for(auto i = 0; i < num_meshes; i++){
            const auto mesh = scene->GetMember<FbxMesh>(i);
            meshes.push_back(ParseMesh(mesh));
        }

        // マテリアルの解析
        const auto num_materials = scene->GetMemberCount<FbxSurfaceMaterial>();
        std::cout << "Materials: " << num_materials << std::endl;
        for(auto i = 0; i < num_materials; i++){
            const auto material = scene->GetMember<FbxSurfaceMaterial>(i);
            this->material.insert(std::make_pair(material->GetName(), ParseMaterial(material)));
        }
    }
    manager->Destroy();

    return true;
}

void Model::ParseSkelton(const fbxsdk::FbxNode* node)
{
    const auto num_children = node->GetChildCount();
    for(auto i = 0; i < num_children; i++){
        auto child = node->GetChild(i);
        ParseSkelton(child, 0, 0, -1);
    }
}

void Model::ParseSkelton(const fbxsdk::FbxNode* node, int depth, int index, int parent_index)
{
    auto attr = node->GetNodeAttribute();
    if(attr->GetAttributeType() == FbxNodeAttribute::eSkeleton){
        if(index == 0){
            parent_index = -1;
        }
        std::cout << "NodeName: " << node->GetName() << " [Depth]=" << depth << " [Index]=" << index << " [Parent]=" << parent_index << std::endl;
        SkeletonJoint joint;
        joint.SetParentIndex(parent_index);
        joint.SetName(node->GetName());
        skeleton->SetJoint(joint);
    }
    auto& joints = skeleton->GetJoints();
    for(auto i = 0; i < node->GetChildCount(); i++){
        ParseSkelton(node->GetChild(i), depth+1, joints.size(), index);
    }
}

std::unique_ptr<Mesh> Model::ParseMesh(const FbxMesh* mesh)
{
    auto result = std::unique_ptr<Mesh>(new Mesh());

    auto node = mesh->GetNode();
    result->node_name = node->GetName();
    std::cout << "NodeName:" << result->node_name.c_str() << std::endl;
    assert(node->GetMaterialCount() == 1);  // 1 mesh : 1 material
    result->material_name = node->GetMaterial(0)->GetName();

    /*
        各要素の EMappingMode に統一性は見られない.
        GetMeshSmoothness() はエクスポータで関与していない可能性がある.
        最適化の指針が得られないことから、すべての要素を eByPolygonVertex で展開.
        その後、頂点レベルで最適化を行う.
     */
    auto positions = GetControlPointsByPolygonVertex(mesh);
    auto normals = GetNormalsByPolygonVertex(mesh);
    auto texcoords0 = GetUVsByPolygonVertex(mesh, 0);
    //
    std::vector<std::array<Weight, 4>> joint_weights;
    const auto num_skin = mesh->GetDeformerCount(FbxDeformer::eSkin);
    if(num_skin > 0){
        assert(num_skin == 1);  // 1 mesh : 1 skin
        GetWeightsByPolygonVertex(mesh, result->joint_names, joint_weights);
    }
    //
    std::vector<Vertex> optimized_vertices;
    std::vector<int>& optimized_indices = result->indices;
    OptimizeMesh(positions, normals, texcoords0, joint_weights, optimized_vertices, optimized_indices);

    const bool has_texcoord0 = texcoords0.size() > 0;
    const bool has_weight = joint_weights.size() > 0;
    result->vertices.reserve(optimized_vertices.size() * 3);
    result->normals.reserve(optimized_vertices.size() * 3);
    if(has_texcoord0){
        result->texcoords0.reserve(optimized_vertices.size() * 2);
    }
    if(has_weight){
        result->joint_indices.reserve(optimized_vertices.size() * 4);
        result->joint_weights.reserve(optimized_vertices.size() * 4);
    }
    for(const auto& v : optimized_vertices){
        result->vertices.push_back(static_cast<float>(v.position[0]));
        result->vertices.push_back(static_cast<float>(v.position[1]));
        result->vertices.push_back(static_cast<float>(v.position[2]));
        result->normals.push_back(static_cast<float>(v.normal[0]));
        result->normals.push_back(static_cast<float>(v.normal[1]));
        result->normals.push_back(static_cast<float>(v.normal[2]));
        if(has_texcoord0){
            result->texcoords0.push_back(static_cast<float>(v.texcoord0[0]));
            result->texcoords0.push_back(static_cast<float>(v.texcoord0[1]));
        }
        if(has_weight){
            result->joint_indices.push_back(static_cast<std::uint8_t>(std::get<0>(v.weights[0])));
            result->joint_indices.push_back(static_cast<std::uint8_t>(std::get<0>(v.weights[1])));
            result->joint_indices.push_back(static_cast<std::uint8_t>(std::get<0>(v.weights[2])));
            result->joint_indices.push_back(static_cast<std::uint8_t>(std::get<0>(v.weights[3])));
            result->joint_weights.push_back(static_cast<float>(std::get<1>(v.weights[0])));
            result->joint_weights.push_back(static_cast<float>(std::get<1>(v.weights[1])));
            result->joint_weights.push_back(static_cast<float>(std::get<1>(v.weights[2])));
            result->joint_weights.push_back(static_cast<float>(std::get<1>(v.weights[3])));
        }
    }
    result->is_skinned_mesh = has_weight;

    return result;
}

void Model::OptimizeMesh(
    const std::vector<FbxDouble3>& positions,
    const std::vector<FbxDouble3>& normals,
    const std::vector<FbxDouble2>& texcoords0,
    const std::vector<std::array<Weight, 4>>& joint_weights,
    std::vector<Vertex>& optimized_vertices,
    std::vector<int>& optimized_indices)
{
    const auto num_pv = positions.size();
    std::vector<Vertex> vertices;
    vertices.reserve(num_pv);

    const bool has_texcoord0 = texcoords0.size() > 0;
    const bool has_weight = joint_weights.size() > 0;

    Vertex vertex;
    for(auto i = 0; i < num_pv; i++){
        vertex.position = positions[i];
        vertex.normal = normals[i];
        vertex.texcoord0 = (has_texcoord0) ? texcoords0[i] : FbxDouble2(0.0, 0.0);
        if(has_weight){
            for(auto j = 0; j < 4; j++){
                vertex.weights[j] = joint_weights[i][j];
            }
        }
        else {
            for (auto j = 0; j < 4; j++) {
                vertex.weights[j] = Weight(0, 0.0);
            }
        }
        vertices.push_back(vertex);
    }

    for(const auto& v : vertices){
        auto it = std::find(optimized_vertices.cbegin(), optimized_vertices.cend(), v);
        if(it == optimized_vertices.cend()){
            optimized_indices.push_back(optimized_vertices.size());
            optimized_vertices.push_back(v);
        }
        else {
            auto index = std::distance(optimized_vertices.cbegin(), it);
            optimized_indices.push_back(index);
        }
    }
}

std::vector<FbxDouble3> Model::GetControlPointsByPolygonVertex(const fbxsdk::FbxMesh* mesh)
{
    std::vector<FbxDouble3> points;

    const auto pv = mesh->GetPolygonVertices();
    const auto num_pv = mesh->GetPolygonVertexCount();
    points.reserve(num_pv);

    for(auto i = 0; i < num_pv; i++){
        const auto cp_index = pv[i];
        const auto& v = mesh->GetControlPointAt(cp_index);
        points.push_back(FbxDouble3(v[0], v[1], v[2]));
    }
    assert(points.size() == num_pv);
    return std::move(points);
}

std::vector<FbxDouble3> Model::GetNormalsByPolygonVertex(const fbxsdk::FbxMesh* mesh)
{
    std::vector<FbxDouble3> normals;
    auto element = mesh->GetElementNormal();
    if(!element)
        return normals;

    const auto mapping = element->GetMappingMode();
    assert((mapping == FbxGeometryElement::eByControlPoint) || (mapping == FbxGeometryElement::eByPolygonVertex));
    const auto reference = element->GetReferenceMode();
    assert((reference == FbxGeometryElement::eDirect) || (reference == FbxGeometryElement::eIndexToDirect));

    const auto num_pv = mesh->GetPolygonVertexCount();

    if(mapping == FbxGeometryElement::eByControlPoint){
        normals.reserve(num_pv);
        const auto pv = mesh->GetPolygonVertices();
        for(auto i = 0; i < num_pv; i++){
            const auto cp_index = pv[i];
            const auto index = (reference == FbxGeometryElement::eDirect) ? cp_index : element->GetIndexArray().GetAt(cp_index);
            const auto normal = element->GetDirectArray().GetAt(index);
            normals.push_back(FbxDouble3(normal[0], normal[1], normal[2]));
        }
    }
    else
    if(mapping == FbxGeometryElement::eByPolygonVertex){
        normals.reserve(num_pv);
        const auto pv = mesh->GetPolygonVertices();
        for(auto i = 0; i < num_pv; i++){
            const auto index = (reference == FbxGeometryElement::eDirect) ? i : element->GetIndexArray().GetAt(i);
            const auto normal = element->GetDirectArray().GetAt(index);
            normals.push_back(FbxDouble3(normal[0], normal[1], normal[2]));
        }
    }
    assert(normals.size() == num_pv);
    return std::move(normals);
}

std::vector<FbxDouble2> Model::GetUVsByPolygonVertex(const fbxsdk::FbxMesh* mesh, int index)
{
    std::vector<FbxDouble2> uvs;
    auto element = mesh->GetElementUV(index);
    if(!element)
        return uvs;

    const auto mapping = element->GetMappingMode();
    assert((mapping == FbxGeometryElement::eByControlPoint) || (mapping == FbxGeometryElement::eByPolygonVertex));
    const auto reference = element->GetReferenceMode();
    assert((reference == FbxGeometryElement::eDirect) || (reference == FbxGeometryElement::eIndexToDirect));

    const auto num_pv = mesh->GetPolygonVertexCount();

    if(mapping == FbxGeometryElement::eByControlPoint){
        uvs.reserve(num_pv);
        const auto pv = mesh->GetPolygonVertices();
        for(auto i = 0; i < num_pv; i++){
            const auto cp_index = pv[i];
            const auto index = (reference == FbxGeometryElement::eDirect) ? cp_index : element->GetIndexArray().GetAt(cp_index);
            const auto uv = element->GetDirectArray().GetAt(index);
            uvs.push_back(FbxDouble2(uv[0], uv[1]));
        }
    }
    else
    if(mapping == FbxGeometryElement::eByPolygonVertex){
        uvs.reserve(num_pv);
        const auto pv = mesh->GetPolygonVertices();
        for(auto i = 0; i < num_pv; i++){
            const auto index = (reference == FbxGeometryElement::eDirect) ? i : element->GetIndexArray().GetAt(i);
            const auto uv = element->GetDirectArray().GetAt(index);
            uvs.push_back(FbxDouble2(uv[0], uv[1]));
        }
    }
    assert(uvs.size() == num_pv);
    return std::move(uvs);
}

std::unordered_map<std::string, std::vector<FbxDouble2>> Model::GetUVsByPolygonVertex(const fbxsdk::FbxMesh* mesh)
{
    std::unordered_map<std::string, std::vector<FbxDouble2>> map;
    const auto num_uv = mesh->GetElementUVCount();
    for(auto i = 0; i < num_uv; i++){
        auto element = mesh->GetElementUV(i);
        if(!element)
            continue;
        map.insert(std::make_pair(element->GetName(), GetUVsByPolygonVertex(mesh, i)));
    }
    return std::move(map);
}

static glm::mat4 convMatrix(FbxAMatrix& m)
{
    auto p = static_cast<double*>(m);
    std::array<float, 16> arr;
    for(auto i = 0; i < 16; i++){
        arr[i] = static_cast<float>(p[i]);
    }
    return glm::make_mat4(arr.data());
}

static FbxAMatrix GetGeometry(FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

void Model::GetWeightsByPolygonVertex(const fbxsdk::FbxMesh* mesh, std::vector<std::string>& joint_names, std::vector<std::array<Weight, 4>>& joint_weights)
{
    FbxSkin* skin = fbxsdk::FbxCast<FbxSkin>(mesh->GetDeformer(0, FbxDeformer::eSkin));

    const auto num_cp = mesh->GetControlPointsCount();
    std::vector<std::vector<Weight>> cp_weights(num_cp);

    const auto num_cluster = skin->GetClusterCount();
    joint_names.reserve(num_cluster);
    for(auto i = 0; i < num_cluster; i++){  // cluster == joint
        const auto cluster = skin->GetCluster(i);
        assert(cluster->GetLinkMode() == FbxCluster::eNormalize);
        joint_names.push_back(cluster->GetLink()->GetName());
        // この関節の影響下にある制御点の数
        const auto num_cp_indices = cluster->GetControlPointIndicesCount();
        for(auto j = 0; j < num_cp_indices; j++){
            // インデクスとウェイトを取得
            auto cp_index = cluster->GetControlPointIndices()[j];
            auto weight = cluster->GetControlPointWeights()[j];
            cp_weights[cp_index].push_back(Weight(i, weight));
        }
        //
        // 情報が重複するがバインドポーズを確実に取得できる箇所がここしかない
        FbxAMatrix ref_global_init_position;
        auto ref_geometry = GetGeometry(mesh->GetNode());
        cluster->GetTransformMatrix(ref_global_init_position);
        ref_global_init_position *= ref_geometry;

        FbxAMatrix cluster_global_init_position;
        cluster->GetTransformLinkMatrix(cluster_global_init_position);

        std::string name = cluster->GetLink()->GetName();
        FbxAMatrix bindpose = cluster_global_init_position * ref_global_init_position;
        FbxAMatrix inv_bindpose = bindpose.Inverse();
        auto joint = skeleton->GetJoint(name);
        if(joint){
            joint->SetInverseBindPose(convMatrix(inv_bindpose));
        }
    }
    // 最大影響度数に応じた調整
    for(auto& weights : cp_weights){
        // 影響度の大きい順にソート
        std::sort(weights.begin(), weights.end(), [](const Weight& lhs, const Weight& rhs) { return std::get<1>(lhs) > std::get<1>(rhs); });
        // 最大影響数を超えたものを捨てる
        while(weights.size() > 4){
            weights.pop_back();
        }
        // 不足分の補填
        while(weights.size() < 4){
            weights.push_back(Weight(0, 0.0));
        }
        // 正規化
        double sum = 0.0;
        for(auto weight : weights){
            sum += std::get<1>(weight);
        }
        for(auto weight : weights){
            std::get<1>(weight) /= sum;
        }
    }
    // 展開
    const auto num_pv = mesh->GetPolygonVertexCount();
    joint_weights.resize(num_pv);
    const auto pv = mesh->GetPolygonVertices();
    for(auto i = 0; i < num_pv; i++){
        const auto cp_index = pv[i];
        for(auto j = 0; j < 4; j++){
            joint_weights[i][j] = cp_weights[cp_index][j];
        }
    }
}

std::unique_ptr<Material> Model::ParseMaterial(const FbxSurfaceMaterial* material)
{
    // TODO: 2016/05/03
    // 現状は、Maya経由でエクスポートされた FBX のみ対応
    // UVSet が "default" となっているものは "map1" に対応する
    auto result = std::unique_ptr<Material>(new Material());

    std::cout << "--- Material Name: " << material->GetName() << std::endl;

    auto impl = GetImplementation(material, FBXSDK_IMPLEMENTATION_HLSL);
    if(!impl){
        impl = GetImplementation(material, FBXSDK_IMPLEMENTATION_CGFX);
    }

    if(impl){
        result->shader = std::move(std::unique_ptr<Material::Shader>(new Material::Shader()));
        auto root_table = impl->GetRootTable();

        std::string shader_filename = root_table->DescAbsoluteURL.Get();
        shader_filename = shader_filename.substr(shader_filename.find_last_of('/') + 1);
        result->shader->filename = shader_filename;

        const auto num_entry = root_table->GetEntryCount();
        for(std::size_t i = 0; i < num_entry; i++){
            auto& entry = root_table->GetEntry(i);
            auto entry_src_type = entry.GetEntryType(true);

            FbxProperty prop;
            if(std::strcmp(FbxPropertyEntryView::sEntryType, entry_src_type) == 0){
                prop = material->FindPropertyHierarchical(entry.GetSource());
                if(!prop.IsValid()){
                    prop = material->RootProperty.FindHierarchical(entry.GetSource());
                }
            }
            else
            if(std::strcmp(FbxConstantEntryView::sEntryType, entry_src_type) == 0){
                prop = impl->GetConstants().FindHierarchical(entry.GetSource());
            }
            //
            if(prop.IsValid()){
                const std::string hierarchical_name = prop.GetHierarchicalName();
                const std::string label = prop.GetLabel();

                if(prop.GetSrcObjectCount<FbxTexture>() > 0){
                    assert(prop.GetSrcObjectCount<FbxLayeredTexture>() == 0);
                    assert(prop.GetSrcObjectCount<FbxProceduralTexture>() == 0);

                    const auto num_src_object = prop.GetSrcObjectCount<FbxFileTexture>();
                    for(auto j = 0; j < num_src_object; j++){
                        auto texture = prop.GetSrcObject<FbxFileTexture>(j);

                        if(hierarchical_name.find("Maya|") == 0){
                            std::string texture_filename = texture->GetFileName();
                            texture_filename = texture_filename.substr(texture_filename.find_last_of('/') + 1);

                            if(label == "DiffuseTexture"){
                                result->shader->diffuse = std::move(std::unique_ptr<Texture>(new Texture()));
                                result->shader->diffuse->filename = texture_filename;
                                result->shader->diffuse->uvset = texture->UVSet.Get();
                                std::cout << "DiffuseTexture: " << texture_filename.c_str() << std::endl;
                            }
                            else
                            if(label == "NormalTexture"){
                                result->shader->normal = std::move(std::unique_ptr<Texture>(new Texture()));
                                result->shader->normal->filename = texture_filename;
                                result->shader->normal->uvset = texture->UVSet.Get();
                                std::cout << "NormalTexture: " << texture_filename.c_str() << std::endl;
                            }
                            else
                            if(label == "SpecularTexture"){
                                result->shader->specular = std::move(std::unique_ptr<Texture>(new Texture()));
                                result->shader->specular->filename = texture_filename;
                                result->shader->specular->uvset = texture->UVSet.Get();
                                std::cout << "SpecularTexture: " << texture_filename.c_str() << std::endl;
                            }
                            else
                            if (label == "FalloffTexture"){
                                result->shader->falloff = std::move(std::unique_ptr<Texture>(new Texture()));
                                result->shader->falloff->filename = texture_filename;
                                result->shader->falloff->uvset = texture->UVSet.Get();
                                std::cout << "FalloffTexture: " << texture_filename.c_str() << std::endl;
                            }
                            else
                            if(label == "ReflectionMapTexture"){
                                result->shader->reflectionmap = std::move(std::unique_ptr<Texture>(new Texture()));
                                result->shader->reflectionmap->filename = texture_filename;
                                result->shader->reflectionmap->uvset = texture->UVSet.Get();
                                std::cout << "ReflectionMapTexture: " << texture_filename.c_str() << std::endl;
                            }
                        }
                        break;  // only one texture.
                    }
                }
                else{
                    // ignore
                    auto prop_data_type = prop.GetPropertyDataType();
                    auto blah = prop_data_type.GetName();
                    std::cout << "["<< hierarchical_name.c_str() << "]" << " DATA TYPE: " << blah << " ";
                    if(prop_data_type == FbxBoolDT){
                        const auto value = prop.Get<FbxBool>();
                        std::cout << value;
                        if (hierarchical_name.find("Maya|") == 0){
                            if(label == "g_bAlphaEnable"){
                                result->is_alpha_enabled = value;
                            }
                            else
                            if(label == "g_bDoubleSided"){
                                result->is_double_side_enabled = value;
                            }
                        }
                    }
                    else
                    if(prop_data_type == FbxIntDT){
                        std::cout << prop.Get<FbxInt>();
                    }
                    else
                    if(prop_data_type == FbxFloatDT){
                        std::cout << prop.Get<FbxFloat>();
                    }
                    else
                    if(prop_data_type == FbxDoubleDT){
                        std::cout << prop.Get<FbxDouble>();
                    }
                    else
                    if((prop_data_type == FbxStringDT) || (prop_data_type == FbxUrlDT) || (prop_data_type == FbxXRefUrlDT)){
                    }
                    else
                    if(prop_data_type == FbxDouble2DT){
                    }
                    else
                    if((prop_data_type == FbxDouble3DT) || (prop_data_type == FbxColor3DT)){
                    }
                    else
                    if((prop_data_type == FbxDouble4DT) || (prop_data_type == FbxColor4DT)){
                    }
                    else
                    if(prop_data_type == FbxDouble4x4DT){
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    else
    if(material->GetClassId().Is(FbxSurfaceLambert::ClassId)){
        auto lambert = FbxCast<FbxSurfaceLambert>(material);

        result->lambert = std::move(std::unique_ptr<Material::Lambert>(new Material::Lambert()));
        result->lambert->ambient = { static_cast<float>(lambert->Ambient.Get()[0]) , static_cast<float>(lambert->Ambient.Get()[1]), static_cast<float>(lambert->Ambient.Get()[2]) };
        result->lambert->diffuse = { static_cast<float>(lambert->Diffuse.Get()[0]) , static_cast<float>(lambert->Diffuse.Get()[1]), static_cast<float>(lambert->Diffuse.Get()[2]) };
        result->lambert->emissive = { static_cast<float>(lambert->Emissive.Get()[0]) , static_cast<float>(lambert->Emissive.Get()[1]), static_cast<float>(lambert->Emissive.Get()[2]) };
        result->lambert->opacity = static_cast<float>(1.0 - lambert->TransparencyFactor.Get());
    }
    else
    if(material->GetClassId().Is(FbxSurfacePhong::ClassId)){
        auto phong = FbxCast<FbxSurfacePhong>(material);

        result->phong = std::move(std::unique_ptr<Material::Phong>(new Material::Phong()));
        result->phong->ambient = { static_cast<float>(phong->Ambient.Get()[0]) , static_cast<float>(phong->Ambient.Get()[1]), static_cast<float>(phong->Ambient.Get()[2]) };
        result->phong->diffuse = { static_cast<float>(phong->Diffuse.Get()[0]) , static_cast<float>(phong->Diffuse.Get()[1]), static_cast<float>(phong->Diffuse.Get()[2]) };
        result->phong->emissive = { static_cast<float>(phong->Emissive.Get()[0]) , static_cast<float>(phong->Emissive.Get()[1]), static_cast<float>(phong->Emissive.Get()[2]) };
        result->phong->specular = { static_cast<float>(phong->Specular.Get()[0]) , static_cast<float>(phong->Specular.Get()[1]), static_cast<float>(phong->Specular.Get()[2]) };
        result->phong->opacity = static_cast<float>(1.0 - phong->TransparencyFactor.Get());
        result->phong->shininess = static_cast<float>(phong->Shininess.Get());
        result->phong->reflectivity = static_cast<float>(phong->ReflectionFactor.Get());
    }
    else{
        assert(0);
    }
    return result;
}

bool Animation::Load(const std::string& filepath)
{
    return false;
}

}