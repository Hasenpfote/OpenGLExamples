#include <cassert>
#include <iostream>
#include <regex>
#include <hasenpfote/math/utils.h>
#include <hasenpfote/math/cmatrix4.h>
#include "model.h"

using namespace hasenpfote::math;

Material::Material()
{
}

Material::~Material()
{
    if(glIsSampler(diffuse_sampler))
        glDeleteSamplers(1, &diffuse_sampler);
}

void Material::Setup(const fbxloader::Material& material)
{
    const auto& shader = material.shader;
    if(shader){
        if(shader->diffuse){
            diffuse_texture_name = std::regex_replace(shader->diffuse->filename, std::regex(R"(\.tga$)"), ".png");
            // 現状は固定.
            glGenSamplers(1, &diffuse_sampler);
            glSamplerParameteri(diffuse_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            //glSamplerParameteri(diffuse_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glSamplerParameteri(diffuse_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameteri(diffuse_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glSamplerParameteri(diffuse_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
    }
    is_alpha_enabled = material.is_alpha_enabled;
    is_double_side_enabled = material.is_double_side_enabled;
}

///

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    if(glIsBuffer(position_buffer_object))
        glDeleteBuffers(1, &position_buffer_object);
    if(glIsBuffer(normal_buffer_object))
        glDeleteBuffers(1, &normal_buffer_object);
    if(glIsBuffer(texcoord0_buffer_object))
        glDeleteBuffers(1, &texcoord0_buffer_object);
    if(glIsBuffer(index_buffer_object))
        glDeleteBuffers(1, &index_buffer_object);
    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);
}

void Mesh::Setup(const fbxloader::Mesh& mesh)
{
    node_name = mesh.node_name;
    material_name = mesh.material_name;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // position
    glGenBuffers(1, &position_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(position_index, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(position_index);

    // normal
    glGenBuffers(1, &normal_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(normal_index, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(normal_index);

    // texcoord0
    if(!mesh.texcoords0.empty()){
        glGenBuffers(1, &texcoord0_buffer_object);
        glBindBuffer(GL_ARRAY_BUFFER, texcoord0_buffer_object);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.texcoords0.size(), mesh.texcoords0.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(texcoord0_index, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(texcoord0_index);
    }

    //
    if(mesh.is_skinned_mesh){
        is_skinned_mesh = true;
        for(const auto& joint_name : mesh.joint_names){
            joint_names.push_back(joint_name);
        }
        glGenBuffers(1, &joint_indices_buffer_object);
        glBindBuffer(GL_ARRAY_BUFFER, joint_indices_buffer_object);
        glBufferData(GL_ARRAY_BUFFER, sizeof(std::uint8_t) * mesh.joint_indices.size(), mesh.joint_indices.data(), GL_STATIC_DRAW);
        glVertexAttribIPointer(joint_indices_index, 4, GL_UNSIGNED_BYTE, 0, nullptr);
        glEnableVertexAttribArray(joint_indices_index);

        glGenBuffers(1, &joint_weights_buffer_object);
        glBindBuffer(GL_ARRAY_BUFFER, joint_weights_buffer_object);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.joint_weights.size(), mesh.joint_weights.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(joint_weights_index, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(joint_weights_index);
    }

    // index
    count = mesh.indices.size();
    glGenBuffers(1, &index_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * count, mesh.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::Draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

///

Model::Model()
{
    vs = nullptr;
    fs = nullptr;
}

Model::~Model()
{
    if(glIsProgram(pipeline))
       glDeleteProgramPipelines(1, &pipeline);
}

void Model::Setup(fbxloader::Model& model, GLuint common_matrices)
{
    // マテリアルのセットアップ
    for(const auto& pair : model.material){
        auto p = std::unique_ptr<Material>(new Material());
        p->Setup(*(pair.second));
        material.insert(std::make_pair(pair.first, std::move(p)));
    }
    // メッシュのセットアップ
    for(const auto& mesh : model.meshes){
        auto p = std::unique_ptr<Mesh>(new Mesh());
        p->Setup(*mesh);
        // 不透明と半透明メッシュを振り分けておく.
        decltype(material)::const_iterator it = material.find(p->GetMaterialName());
        if(it != material.cend()){
            if(it->second->IsAlphaEnabled()){
                transparent_meshes.push_back(std::move(p));
            }
            else {
                this->opaque_meshes.push_back(std::move(p));
            }
        }
    }
    // スケルトンのセットアップ
    if(model.skeleton){
        skeleton = std::move(model.skeleton);
        skeleton->BuildByInvBindPose();
    }

    this->common_matrices = common_matrices;

    auto& rm = System::GetMutableInstance().GetResourceManager();

    vs = rm.GetResource<ShaderProgram>("assets/shaders/simple.vs");
    assert(vs);
    fs = rm.GetResource<ShaderProgram>("assets/shaders/simple.fs");
    assert(fs);

    glGenProgramPipelines(1, &pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vs->GetProgram());
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fs->GetProgram());
}

void Model::DrawOpaqueMeshes(const hasenpfote::math::CMatrix4& model)
{
    assert(vs);
    assert(fs);

    GLboolean cf;
    glGetBooleanv(GL_CULL_FACE, &cf);
    GLint cfm;
    glGetIntegerv(GL_CULL_FACE_MODE, &cfm);
    GLint ff;
    glGetIntegerv(GL_FRONT_FACE, &ff);

    glCullFace(GL_BACK);

    auto vs_program = vs->GetProgram();
    //glGetProgramPipelineiv(pipeline, GL_VERTEX_SHADER, &vs_program);
    //GLuint ub_index = glGetUniformBlockIndex(vs_program, "CommonMatrices");
    glBindBufferBase(GL_UNIFORM_BUFFER, vs->GetUniformBlockIndex("CommonMatrices"), common_matrices);

    glProgramUniformMatrix4fv(vs_program, vs->GetUniformLocation("matWorld"), 1, GL_FALSE, static_cast<const GLfloat*>(model));

    auto loc1 = vs->GetUniformLocation("jointPalette");
    auto loc2 = vs->GetUniformLocation("isSkinnedMesh");

    auto fs_program = fs->GetProgram();
    glProgramUniform1i(fs_program, fs->GetUniformLocation("texture"), 0);

    glUseProgram(0);
    glBindProgramPipeline(pipeline);

    auto& rm = System::GetConstInstance().GetResourceManager();

    for(const auto& mesh : opaque_meshes)
    {
        decltype(material)::const_iterator it = material.find(mesh->GetMaterialName());
        if(it != material.cend())
        {
            if(it->second->IsDoubleSideEnabled())
            {
                glDisable(GL_CULL_FACE);
            }
            else
            {
                glEnable(GL_CULL_FACE);
            }
            //
#if 0
            const auto difuse_texture_name = it->second->GetDiffuseTextureName();
#else       // 暫定
            const auto difuse_texture_name = "assets/textures/" + it->second->GetDiffuseTextureName();
#endif
            if(!difuse_texture_name.empty())
            {
                GLuint texture = 0;
                auto resource = rm.GetResource<Texture>(difuse_texture_name);
                if(resource != nullptr)
                    texture = resource->GetTexture();

                if(texture > 0)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glBindSampler(0, it->second->GetDiffuseSampler());
                }
                else
                {
                    glActiveTexture(GL_TEXTURE0);
                }
            }
            //
            if(mesh->IsSkinnedMesh())
            {
                auto palette_count = 0;
                CMatrix4 mat_final;
                auto& joints_name = mesh->GetJointNames();
                for(const auto& joint_name : joints_name)
                {
                    auto joint = skeleton->GetJoint(joint_name);
                    if(joint)
                    {
                        mat_final = joint->GetGlobalPose() * joint->GetInverseBindPose();
                    }
                    palette[palette_count] = mat_final;
                    palette_count++;
                }
                glProgramUniformMatrix4fv(vs_program, loc1, palette_count, GL_FALSE, reinterpret_cast<float*>(palette.data()));
                glProgramUniform1i(vs_program, loc2, GL_TRUE);
            }
            else
            {
                glProgramUniform1i(vs_program, loc2, GL_FALSE);
            }
            mesh->Draw();
        }
    }

    glActiveTexture(GL_TEXTURE0);

    glBindProgramPipeline(0);

    if(cf)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
    glFrontFace(ff);
    glCullFace(cfm);
}

void Model::DrawTransparentMeshes(const hasenpfote::math::CMatrix4& model)
{
    assert(vs);
    assert(fs);

    GLboolean cf;
    glGetBooleanv(GL_CULL_FACE, &cf);
    GLint cfm;
    glGetIntegerv(GL_CULL_FACE_MODE, &cfm);
    GLint ff;
    glGetIntegerv(GL_FRONT_FACE, &ff);

    glCullFace(GL_BACK);

    auto vs_program = vs->GetProgram();
    //glGetProgramPipelineiv(pipeline, GL_VERTEX_SHADER, &vs_program);

    GLuint ub_index = glGetUniformBlockIndex(vs_program, "CommonMatrices");
    glBindBufferBase(GL_UNIFORM_BUFFER, ub_index, common_matrices);

    glProgramUniformMatrix4fv(vs_program, vs->GetUniformLocation("matWorld"), 1, GL_FALSE, static_cast<const GLfloat*>(model));

    auto loc1 = vs->GetUniformLocation("jointPalette");
    auto loc2 = vs->GetUniformLocation("isSkinnedMesh");

    auto fs_program = fs->GetProgram();
    glProgramUniform1i(fs_program, fs->GetUniformLocation("texture"), 0);

    glUseProgram(0);
    glBindProgramPipeline(pipeline);

    auto& rm = System::GetConstInstance().GetResourceManager();

    for(const auto& mesh : transparent_meshes)
    {
        decltype(material)::const_iterator it = material.find(mesh->GetMaterialName());
        if(it != material.cend())
        {
            if(it->second->IsDoubleSideEnabled())
            {
                glDisable(GL_CULL_FACE);
            }
            else
            {
                glEnable(GL_CULL_FACE);
            }
            //
#if 0
            const auto difuse_texture_name = it->second->GetDiffuseTextureName();
#else       // 暫定
            const auto difuse_texture_name = "assets/textures/" + it->second->GetDiffuseTextureName();
#endif
            if(!difuse_texture_name.empty())
            {
                GLuint texture = 0;
                auto resource = rm.GetResource<Texture>(difuse_texture_name);
                if (resource != nullptr)
                    texture = resource->GetTexture();

                if(texture > 0)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glBindSampler(0, it->second->GetDiffuseSampler());
                }
                else
                {
                    glActiveTexture(GL_TEXTURE0);
                }
            }
            //
            if(mesh->IsSkinnedMesh())
            {
                auto palette_count = 0;
                CMatrix4 mat_final;
                auto& joints_name = mesh->GetJointNames();
                for (const auto& joint_name : joints_name)
                {
                    auto joint = skeleton->GetJoint(joint_name);
                    if(joint)
                    {
                        mat_final = joint->GetGlobalPose() * joint->GetInverseBindPose();
                    }
                    palette[palette_count] = mat_final;
                    palette_count++;
                }
                glProgramUniformMatrix4fv(vs_program, loc1, palette_count, GL_FALSE, reinterpret_cast<float*>(palette.data()));
                glProgramUniform1i(vs_program, loc2, GL_TRUE);
            }
            else
            {
                glProgramUniform1i(vs_program, loc2, GL_FALSE);
            }
            mesh->Draw();
        }
    }

    glActiveTexture(GL_TEXTURE0);

    glBindProgramPipeline(0);

    if(cf)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
    glFrontFace(ff);
    glCullFace(cfm);
}

static void render_basis(float length)
{
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.5f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(length, 0.0f, 0.0f);

    glColor3f(0.5f, 1.0f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, length, 0.0f);

    glColor3f(0.5f, 0.5f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, length);
    glEnd();
}

void Model::DrawSkeleton()
{
    if(!skeleton)
        return;

    auto& joints = skeleton->GetJoints();
    for(auto& joint : joints){
        glPushMatrix();
        glMultMatrixf(static_cast<GLfloat*>(joint.GetGlobalPose()));
        render_basis(10.0f);
        glPopMatrix();
    }
}

void Model::Update(double dt)
{
    skeleton->BuildByInvBindPose();

    constexpr auto max_limit = ConvertDegreesToRadians(45.0f);
    constexpr auto min_limit = ConvertDegreesToRadians(-45.0f);

    static float s = ConvertDegreesToRadians(10.0f);
    static float t = 0.0f;
    if((t > max_limit) || (t < min_limit)){
        s = -s;
    }

    t += s * dt;

    CMatrix4 pose = CMatrix4::RotationY(t);
    {
        auto target = skeleton->GetJoint("Character1_LeftArm");
        if(target){
            auto local = target->GetLocalPose();
            target->SetLocalPose(local * pose);
        }
    }
    skeleton->LocalToGlobal();
}

