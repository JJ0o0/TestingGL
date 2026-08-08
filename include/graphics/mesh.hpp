#pragma once

#include <graphics/vertex.hpp>

#include <cstdint>
#include <vector>

class Mesh {
    public:
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void Draw() const;
    private:
        uint32_t m_vbo = 0;
        uint32_t m_ebo = 0;
        uint32_t m_vao = 0;

        int m_indexCount = 0;

        void setupAttributes();
};
