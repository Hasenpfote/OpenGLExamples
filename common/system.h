#pragma once
#include <memory>
#include "singleton.h"
#include "shader.h"
#include "texture.h"
#include "custom_camera.h"

class System final : public Singleton<System>
{
    friend class Singleton<System>;
private:
    System();

public:
    ~System();

public:
    ShaderManager& GetShaderManager(){ return *shader_man; }
    const ShaderManager& GetShaderManager() const { return *shader_man; }
    TextureManager& GetTextureManager(){ return *texture_man; }
    const TextureManager& GetTextureManager() const { return *texture_man; }
    CustomCamera& GetCamera(){ return *camera; }
    const CustomCamera& GetCamera() const { return *camera; }

private:
    std::unique_ptr<ShaderManager> shader_man;
    std::unique_ptr<TextureManager> texture_man;
    std::unique_ptr<CustomCamera> camera;
};