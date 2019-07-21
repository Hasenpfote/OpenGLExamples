cmake_minimum_required(VERSION 3.5)

project(OpenGLExamples)

### Debug
#message("CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})
#message("CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR})
#message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})
#message("CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})

### Additional cmake files.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
message(STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})

### Check if supported platforms and compilers.
if(WIN32)
    if(MSVC)
    elseif()
        message(FATAL_ERROR "You are using an unsupported Windows compiler!")
    endif()
elseif(UNIX)
    option(FORCE_32BIT_BUILD "Force a 32bit compile on 64bit" OFF)
else()
    message(FATAL_ERROR "You are on an unsupported platform!")
endif()

### OpenGL
find_package(OpenGL REQUIRED)

### External
add_subdirectory(external)

### Examples
set(examples
    "AdaptiveTerrainTessellation"
    "BillboardBeam"
    "Bloom"
    "Demo1"
    "Dithering"
    "FBXLoader"
    "KawaseBlur"
    "LightStreak"
    "Quad"
    "RadialBlur"
    "SRGBChecker"
    "Texture2DArray"
    "ToneMapping"
)

foreach(example IN LISTS examples)
    message(STATUS "example: ${example}")
    add_subdirectory(${example})
endforeach()