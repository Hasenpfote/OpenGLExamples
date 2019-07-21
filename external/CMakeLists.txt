### glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

### glew
add_subdirectory(glew/build/cmake)
set(GLEW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glew/include" CACHE INTERNAL "")

### stb
set(STB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stb" CACHE INTERNAL "")

### tinyexr
set(TINYEXR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tinyexr" CACHE INTERNAL "")

### hasenpfote
add_subdirectory(hasenpfote)