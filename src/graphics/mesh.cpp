#include <graphics/mesh.hpp>
#include <glad/gl.h>

#include <cstddef>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices)
    : m_indexCount(indices.size()) {
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    setupAttributes();

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo),
      m_indexCount(other.m_indexCount) {
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (&other == this) return *this;

    glDeleteBuffers(1, &m_ebo);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);

    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_ebo = other.m_ebo;
    m_indexCount = other.m_indexCount;

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_indexCount = 0;

    return *this;
}

void Mesh::Draw() const {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupAttributes() {
    // POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);
}
