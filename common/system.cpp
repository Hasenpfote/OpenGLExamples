#include <iostream>
#include <sstream>
#include <GL/glew.h>
#include "logger.h"
//#include "shader.h"
//#include "texture.h"
//#include "custom_camera.h"
#include "system.h"

System::System()
{
    LOG_D(__func__);
    shader_man = std::unique_ptr<ShaderManager>(new ShaderManager());
    texture_man = std::unique_ptr<TextureManager>(new TextureManager());
    camera = std::unique_ptr<CustomCamera>(new CustomCamera());
}

System::~System()
{
    LOG_D(__func__);
}