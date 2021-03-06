cmake_minimum_required(VERSION 3.5)

project(FBXLoader)

include(template)

###
message("CMAKE_GENERATOR: " ${CMAKE_GENERATOR})

set(FBXSDK_VERSION "2017.0.1")
set(FBXSDK_SDKS "D:/Autodesk/FBX/FBX SDK")
find_package(FBX REQUIRED)
if(NOT FBXSDK_FOUND)
  message(FATAL_ERROR
    "Can't find FBX SDK in either:\n"
    " - Windows: ${FBXSDK_WINDOWS_ROOT}\n"
  )
endif()

enable_language(CXX)

### Compiler settings.
include(compiler_settings)

### The relative path from `CMAKE_CURRENT_SOURCE_DIR`.
set(SRC_DIR "src")

### Fetch files.
file(GLOB_RECURSE HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ${SRC_DIR}/*.h)
file(GLOB_RECURSE SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ${SRC_DIR}/*.cpp)
set(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

### Fetch common files.
set(COMMON_DIR "../common")
file(GLOB_RECURSE COMMON_HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ${COMMON_DIR}/*.h)
file(GLOB_RECURSE COMMON_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ${COMMON_DIR}/*.cpp)

### Remove deprecated files.
set(filter "[.]*/deprecated/[.]*")
remove_by_filter("${COMMON_HEADER_FILES}" ${filter} COMMON_HEADER_FILES)
remove_by_filter("${COMMON_SOURCE_FILES}" ${filter} COMMON_SOURCE_FILES)

### Remove files.
set(COMMON_IMGUI_DIR "../common/imgui")

set(HEADER_FILES_TO_REMOVE
    "${COMMON_IMGUI_DIR}/imgui_impl_opengl3.h"
    "${COMMON_IMGUI_DIR}/imgui_impl_glfw.h"
)
remove_sublist("${COMMON_HEADER_FILES}" "${HEADER_FILES_TO_REMOVE}" COMMON_HEADER_FILES)

set(SOURCE_FILES_TO_REMOVE
    "${COMMON_IMGUI_DIR}/imgui_impl_opengl3.cpp"
    "${COMMON_IMGUI_DIR}/imgui_impl_glfw.cpp"
)
remove_sublist("${COMMON_SOURCE_FILES}" "${SOURCE_FILES_TO_REMOVE}" COMMON_SOURCE_FILES)
######

set(ALL_COMMON_FILES ${COMMON_HEADER_FILES} ${COMMON_SOURCE_FILES})
set(ALL_FILES ${ALL_FILES} ${ALL_COMMON_FILES})

### Define a grouping for source files in IDE project generation.
if(WIN32 AND MSVC)
    include(utils)
    do_grouping_for_files("Header Files" ${SRC_DIR} "${HEADER_FILES}")
    do_grouping_for_files("Source Files" ${SRC_DIR} "${SOURCE_FILES}")
    do_grouping_for_files("Common" ${COMMON_DIR} "${ALL_COMMON_FILES}")
endif()

### Targets.
add_executable(${PROJECT_NAME} ${ALL_FILES})
if(FORCE_32BIT_BUILD)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

### Debugger working directory.
if(WIN32 AND MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${OPENGL_INCLUDE_DIR}
    PUBLIC ${GLEW_INCLUDE_DIR}
    PUBLIC ${STB_INCLUDE_DIR}
    PUBLIC ${TINYEXR_INCLUDE_DIR}
    PUBLIC ${FBXSDK_INCLUDE_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    ${OPENGL_gl_LIBRARY}
    glfw
    glew_s
    glm
    hasenpfote
    debug ${FBXSDK_LIBRARY_DEBUG}
    optimized ${FBXSDK_LIBRARY}
)

### Install.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/Debug CONFIGURATIONS Debug)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/Release CONFIGURATIONS Release)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/MinSizeRel CONFIGURATIONS MinSizeRel)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/RelWithDebInfo CONFIGURATIONS RelWithDebInfo)