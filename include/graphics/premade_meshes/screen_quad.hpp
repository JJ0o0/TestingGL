#pragma once

#include <graphics/mesh.hpp>
#include <graphics/vertex.hpp>

#include <memory>
#include <vector>

inline std::shared_ptr<Mesh> CreateScreenQuad() {
    std::vector<Vertex> vertices {
        Vertex {
            .Position = {-1.0f, -1.0f, 0.0f},
            .TexCoords = {0.0f, 0.0f}
        },
        Vertex {
            .Position = { 1.0f, -1.0f, 0.0f},
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = { 1.0f,  1.0f, 0.0f},
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {-1.0f,  1.0f, 0.0f},
            .TexCoords = {0.0f, 1.0f}
        }
    };

    std::vector<uint32_t> indices {
        0, 1, 2,
        2, 3, 0
    };

    return std::make_shared<Mesh>(vertices, indices);
}
