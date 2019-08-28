
### Remove sublist from a list.
function(remove_sublist src_list sub_list dst_list)

    set(indices)
    foreach(sub_list_elem IN LISTS sub_list)
        list(FIND src_list ${sub_list_elem} index)
        if(${index} GREATER -1)
            list(APPEND indices ${index})
        endif()
    endforeach()

    if(indices)
        list(REMOVE_AT src_list ${indices})
    endif()

    set(${dst_list} ${src_list} PARENT_SCOPE)

endfunction()

### Remove sublist from a list by filter.
function(remove_by_filter src_list filter dst_list)

    set(sub_list)
    foreach(src_list_elem IN LISTS src_list)
        if(${src_list_elem} MATCHES ${filter})
            list(APPEND sub_list ${src_list_elem})
        endif()
    endforeach()

    remove_sublist("${src_list}" "${sub_list}" temp_list)

    set(${dst_list} ${temp_list} PARENT_SCOPE)

endfunction()

### 
function(make_simple_example_project project_name uses_imgui)

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

    ### Remove imgui_impl files if needed.
    if(NOT uses_imgui)
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
    endif()

    set(ALL_COMMON_FILES ${COMMON_HEADER_FILES} ${COMMON_SOURCE_FILES})
    set(ALL_FILES ${ALL_FILES} ${ALL_COMMON_FILES})

    ### imgui files.
    if(uses_imgui)
        set(IMGUI_DIR "../external/imgui")
        file(GLOB IMGUI_HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ${IMGUI_DIR}/*.h)
        file(GLOB IMGUI_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" ${IMGUI_DIR}/*.cpp)
        set(ALL_IMGUI_FILES ${IMGUI_HEADER_FILES} ${IMGUI_SOURCE_FILES})
        set(ALL_FILES ${ALL_FILES} ${ALL_IMGUI_FILES})
    endif()

    ### Define a grouping for source files in IDE project generation.
    if(WIN32 AND MSVC)
        include(utils)
        do_grouping_for_files("Header Files" ${SRC_DIR} "${HEADER_FILES}")
        do_grouping_for_files("Source Files" ${SRC_DIR} "${SOURCE_FILES}")
        do_grouping_for_files("common" ${COMMON_DIR} "${ALL_COMMON_FILES}")
        if(uses_imgui)
            do_grouping_for_files("imgui" ${IMGUI_DIR} "${ALL_IMGUI_FILES}")
        endif()
    endif()

    if(uses_imgui)
        add_definitions(-DUSE_IMGUI)
    endif()

    ### Targets.
    add_executable(${project_name} ${ALL_FILES})
    if(FORCE_32BIT_BUILD)
        set_target_properties(${project_name} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    endif()

    ### Debugger working directory.
    if(WIN32 AND MSVC)
        set_target_properties(${project_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    target_include_directories(
        ${project_name}
        PUBLIC ${OPENGL_INCLUDE_DIR}
        PUBLIC ${GLEW_INCLUDE_DIR}
        PUBLIC ${STB_INCLUDE_DIR}
        PUBLIC ${TINYEXR_INCLUDE_DIR}
    )

    target_link_libraries(
        ${project_name}
        ${OPENGL_gl_LIBRARY}
        glfw
        glew_s
        glm
        hasenpfote
    )

    ### Install.
    install(TARGETS ${project_name} RUNTIME DESTINATION bin/Debug CONFIGURATIONS Debug)
    install(TARGETS ${project_name} RUNTIME DESTINATION bin/Release CONFIGURATIONS Release)
    install(TARGETS ${project_name} RUNTIME DESTINATION bin/MinSizeRel CONFIGURATIONS MinSizeRel)
    install(TARGETS ${project_name} RUNTIME DESTINATION bin/RelWithDebInfo CONFIGURATIONS RelWithDebInfo)

endfunction()