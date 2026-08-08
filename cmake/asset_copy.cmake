include_guard(GLOBAL)

function(copy_assets TARGET_NAME)
    if(NOT TARGET "${TARGET_NAME}")
        message(FATAL_ERROR "copy_assets: target '${TARGET_NAME}' doesn't exist")
    endif()

    set(ASSETS_SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/assets")
    set(ASSETS_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/assets")

    if(NOT EXISTS "${ASSETS_SOURCE_DIRECTORY}")
        message(FATAL_ERROR "Assets directory not found at '${ASSETS_SOURCE_DIRECTORY}'")
    endif()

    set(ASSETS_TARGET "${TARGET_NAME}_assets")

    add_custom_target("${ASSETS_TARGET}" ALL
        COMMAND
            "${CMAKE_COMMAND}" -E make_directory
            "${ASSETS_OUTPUT_DIRECTORY}"
        COMMAND
            "${CMAKE_COMMAND}" -E copy_directory_if_different
            "${ASSETS_SOURCE_DIRECTORY}"
            "${ASSETS_OUTPUT_DIRECTORY}"
        COMMENT "Copying assets for '${TARGET_NAME}'"
        VERBATIM
    )

    add_dependencies(
        "${TARGET_NAME}"
        "${ASSETS_TARGET}"
    )
endfunction()
