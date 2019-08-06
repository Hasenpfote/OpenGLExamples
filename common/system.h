#pragma once
#include <memory>
#include "singleton.h"
#include "resource.h"
#include "texture.h"
#include "shader.h"
#include "custom_camera.h"

class System final : public Singleton<System>
{
    friend class Singleton<System>;
private:
    System();

public:
    ~System();

public:
    common::DefaultResourceManager& GetResourceManager() { return *rm_; }
    const common::DefaultResourceManager& GetResourceManager() const { return *rm_; }

    CustomCamera& GetCamera(){ return *camera_; }
    const CustomCamera& GetCamera() const { return *camera_; }

private:
    std::unique_ptr<common::DefaultResourceManager> rm_;
    std::unique_ptr<CustomCamera> camera_;
};