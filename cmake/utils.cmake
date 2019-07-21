
#-------------------------------------------------------------------------------
# Define a grouping for source files in IDE project generation.
#-------------------------------------------------------------------------------
function(do_grouping_for_files GROUP_NAME SOURCES_DIR FILE_PATHS)
    foreach(FILE_PATH IN LISTS FILE_PATHS)
        get_filename_component(FILTER ${FILE_PATH} DIRECTORY)
        string(REGEX REPLACE "^${SOURCES_DIR}" "${GROUP_NAME}" FILTER ${FILTER})
        if(WIN32)
            string(REPLACE "/" "\\" FILTER ${FILTER})
        endif()
        source_group("${FILTER}" FILES ${FILE_PATH})
    endforeach()
endfunction()
