#pragma once

#include <core/error_handling.hpp>
#include <graphics/vertex.hpp>
#include <graphics/mesh.hpp>

#include <cstdint>
#include <memory>
#include <vector>

inline std::shared_ptr<Mesh> CreateQuad(float size = 1.0f) {
    CheckError(size > 0.0f, "Mesh Creation", "Tried to create Quad with invalid size");

    const float half = size * 0.5f;
    const glm::vec3 normal{0.0f, 0.0f, 1.0f};
    std::vector<Vertex> vertices {
        Vertex {
            .Position = {half, half, 0.0f},
            .Normal = normal,
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {half, -half, 0.0f},
            .Normal = normal,
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = {-half, half, 0.0f},
            .Normal = normal,
            .TexCoords = {0.0f, 1.0f}
        },
        Vertex {
            .Position = {-half, -half, 0.0f},
            .Normal = normal,
            .TexCoords = {0.0f, 0.0f}
        },
    };

    std::vector<uint32_t> indices {
        0, 2, 1,
        1, 2, 3
    };

    return std::make_shared<Mesh>(vertices, indices);
}
