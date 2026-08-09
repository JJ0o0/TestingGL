#include <graphics/cubemap.hpp>

#include <glad/gl.h>
#include <utility>

Cubemap::Cubemap(uint32_t size, int internalFormat, int format, int type, bool mipmaps)
    : m_size(size) {
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    for (uint32_t face = 0; face < 6; ++face) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            0,
            internalFormat,
            size, size,
            0,
            format,
            type,
            nullptr
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    if (mipmaps) glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

Cubemap::~Cubemap() { if (m_id) glDeleteTextures(1, &m_id); }

Cubemap::Cubemap(Cubemap&& other) noexcept
    : m_id(std::exchange(other.m_id, 0)) {
}

Cubemap& Cubemap::operator=(Cubemap&& other) noexcept {
    if (this == &other) return *this;

    if (m_id) glDeleteTextures(1, &m_id);
    m_id = std::exchange(other.m_id, 0);

    return *this;
}

void Cubemap::Bind(int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}
