﻿#include <iostream>
#include <sstream>
#include <GL/glew.h>
#include "logger.h"
#include "system.h"

System::System()
{
    LOG_D(__func__);
    rm_ = std::make_unique<common::DefaultResourceManager>();
    camera_ = std::make_unique<CustomCamera>();
}

System::~System()
{
    LOG_D(__func__);
}