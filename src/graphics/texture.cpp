#include <graphics/texture.hpp>

#include <platform/image_loading.hpp>

#include <glad/gl.h>
#include <utility>

Texture::Texture(const std::filesystem::path& path, TextureFormat format) {
    ImageData image = LoadImage(path);

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int targetFormat = GL_RGB;
    switch (format) {
        case TextureFormat::RGBA8: targetFormat = GL_RGBA8; break;
        case TextureFormat::SRGBA8: targetFormat = GL_SRGB8_ALPHA8; break;
    }

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        targetFormat,
        image.Width, image.Height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image.Pixels.data()
    );

    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture() {
    if (m_id) glDeleteTextures(1, &m_id);
}

Texture::Texture(Texture&& other) noexcept
    : m_id(std::exchange(other.m_id, 0)) {
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this == &other) return *this;

    if (m_id) glDeleteTextures(1, &m_id);
    m_id = std::exchange(other.m_id, 0);

    return *this;
}

void Texture::Bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
