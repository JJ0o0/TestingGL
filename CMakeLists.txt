cmake_minimum_required(VERSION 3.20)
project(
    TestingGL
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/dependencies.cmake)
include(cmake/asset_copy.cmake)

add_executable(${CMAKE_PROJECT_NAME}
    "src/main.cpp"
    "src/gl.c"
    "src/stb_image.cpp"

    "src/core/application.cpp"
    "src/core/game.cpp"
    "src/core/input.cpp"

    "src/graphics/camera.cpp"
    "src/graphics/mesh.cpp"
    "src/graphics/shader.cpp"
    "src/graphics/texture.cpp"
    "src/graphics/model_loader.cpp"

    "src/platform/window.cpp"
    "src/platform/default_resources.cpp"
)

copy_assets(TestingGL)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        "include/"
        "external/"
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        glfw
        glm::glm
        fastgltf::fastgltf
)

set_target_properties(${CMAKE_PROJECT_NAME}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>"
)
