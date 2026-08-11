#include <graphics/shadow_map.hpp>

#include <core/error_handling.hpp>

#include <glad/gl.h>

ShadowMap::ShadowMap(uint32_t width, uint32_t height)
    : m_width(width), m_height(height) {
    // DEPTH TEXTURE
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT24,
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // FRAMEBUFFER
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        m_depthTexture,
        0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    CheckError(
        glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "Shadow Map Framebuffer", "Failed to create shadow map framebuffer"
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ShadowMap::~ShadowMap() {
    if (!m_depthTexture) glDeleteTextures(1, &m_depthTexture);
    if (!m_framebuffer) glDeleteFramebuffers(1, &m_framebuffer);
}

void ShadowMap::Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer); }
void ShadowMap::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void ShadowMap::BindTexture(uint32_t unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
}
