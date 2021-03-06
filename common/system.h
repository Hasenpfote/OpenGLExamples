﻿#pragma once
#include <memory>
#include "singleton.h"
#include "resource.h"
#include "render/simple_camera.h"

namespace common
{

class System final : public common::Singleton<System>
{
    friend class common::Singleton<System>;
private:
    System();

public:
    ~System();

public:
    common::DefaultResourceManager& GetResourceManager(){ return *rm_; }
    const common::DefaultResourceManager& GetResourceManager() const { return *rm_; }

    common::render::SimpleCamera& GetCamera(){ return *camera_; }
    const common::render::SimpleCamera& GetCamera() const { return *camera_; }

private:
    std::unique_ptr<common::DefaultResourceManager> rm_;
    std::unique_ptr<common::render::SimpleCamera> camera_;
};

}   // namespace common