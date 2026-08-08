#pragma once

#include <core/error_handling.hpp>
#include <graphics/vertex.hpp>
#include <graphics/mesh.hpp>

#include <cstdint>
#include <memory>
#include <vector>

inline std::shared_ptr<Mesh> CreateCube(float size = 1.0f) {
    CheckError(size > 0.0f, "Mesh Creation", "Tried to create Cube with invalid size");

    const float half = size * 0.5f;
    std::vector<Vertex> vertices {
        // FRONT
        Vertex {
            .Position = {-half, -half, half},
            .Normal = {0.0f, 0.0f, 1.0f},
            .TexCoords = {0.0f, 0.0f}
        },
        Vertex {
            .Position = {half, -half, half},
            .Normal = {0.0f, 0.0f, 1.0f},
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = {half, half, half},
            .Normal = {0.0f, 0.0f, 1.0f},
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {-half, half, half},
            .Normal = {0.0f, 0.0f, 1.0f},
            .TexCoords = {0.0f, 1.0f}
        },

        // BACK
        Vertex {
            .Position = {half, -half, -half},
            .Normal = {0.0f, 0.0f, -1.0f},
            .TexCoords = {0.0f, 0.0f}
        },
        Vertex {
            .Position = {-half, -half, -half},
            .Normal = {0.0f, 0.0f, -1.0f},
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = {-half, half, -half},
            .Normal = {0.0f, 0.0f, -1.0f},
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {half, half, -half},
            .Normal = {0.0f, 0.0f, -1.0f},
            .TexCoords = {0.0f, 1.0f}
        },

        // RIGHT
        Vertex {
            .Position = {half, -half, half},
            .Normal = {1.0f, 0.0f, 0.0f},
            .TexCoords = {0.0f, 0.0f}
        },
        Vertex {
            .Position = {half, -half, -half},
            .Normal = {1.0f, 0.0f, 0.0f},
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = {half, half, -half},
            .Normal = {1.0f, 0.0f, 0.0f},
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {half, half, half},
            .Normal = {1.0f, 0.0f, 0.0f},
            .TexCoords = {0.0f, 1.0f}
        },

        // LEFT
        Vertex {
            .Position = {-half, -half, -half},
            .Normal = {-1.0f, 0.0f, 0.0f},
            .TexCoords = {0.0f, 0.0f}
        },
        Vertex {
            .Position = {-half, -half, half},
            .Normal = {-1.0f, 0.0f, 0.0f},
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = {-half, half, half},
            .Normal = {-1.0f, 0.0f, 0.0f},
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {-half, half, -half},
            .Normal = {-1.0f, 0.0f, 0.0f},
            .TexCoords = {0.0f, 1.0f}
        },

        // TOP
        Vertex {
            .Position = {-half, half, half},
            .Normal = {0.0f, 1.0f, 0.0f},
            .TexCoords = {0.0f, 0.0f}
        },
        Vertex {
            .Position = {half, half, half},
            .Normal = {0.0f, 1.0f, 0.0f},
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = {half, half, -half},
            .Normal = {0.0f, 1.0f, 0.0f},
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {-half, half, -half},
            .Normal = {0.0f, 1.0f, 0.0f},
            .TexCoords = {0.0f, 1.0f}
        },

        // BOTTOM
        Vertex {
            .Position = {-half, -half, -half},
            .Normal = {0.0f, -1.0f, 0.0f},
            .TexCoords = {0.0f, 0.0f}
        },
        Vertex {
            .Position = {half, -half, -half},
            .Normal = {0.0f, -1.0f, 0.0f},
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = {half, -half, half},
            .Normal = {0.0f, -1.0f, 0.0f},
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {-half, -half, half},
            .Normal = {0.0f, -1.0f, 0.0f},
            .TexCoords = {0.0f, 1.0f}
        },
    };

    std::vector<uint32_t> indices {
        // FRONT
        0, 1, 2,
        2, 3, 0,

        // BACK
        4, 5, 6,
        6, 7, 4,

        // RIGHT
        8, 9, 10,
        10, 11, 8,

        // LEFT
        12, 13, 14,
        14, 15, 12,

        // TOP
        16, 17, 18,
        18, 19, 16,

        // BOTTOM
        20, 21, 22,
        22, 23, 20
    };

    return std::make_shared<Mesh>(vertices, indices);
}
