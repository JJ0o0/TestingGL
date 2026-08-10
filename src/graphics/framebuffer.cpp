#include <graphics/framebuffer.hpp>

#include <core/logging.hpp>

#include <glad/gl.h>

Framebuffer::Framebuffer(uint32_t width, uint32_t height)
    : m_width(width),
      m_height(height) {
    invalidate();
}

Framebuffer::~Framebuffer() {
    destroy();
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0 || (width == m_width && height == m_height)) return;

    m_width = width;
    m_height = height;

    destroy();
    invalidate();
}

void Framebuffer::invalidate() {
    glGenFramebuffers(1, &m_id);
    glBindFramebuffer(GL_FRAMEBUFFER,m_id);

    // HDR
    m_colorAttachment = std::make_unique<Texture>(
        m_width, m_height,
        GL_RGBA16F,
        GL_RGBA,
        GL_FLOAT
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_colorAttachment->GetID(),
        0
    );

    // DEPTH
    glGenRenderbuffers(1, &m_depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH_COMPONENT24,
        m_width, m_height
    );

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        m_depthRBO
    );

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LogError("Framebuffer is incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy() {
    m_colorAttachment.reset();

    if (m_depthRBO) {
        glDeleteRenderbuffers(1, &m_depthRBO);
        m_depthRBO = 0;
    }

    if (m_id) {
        glDeleteFramebuffers(1, &m_id);
        m_id = 0;
    }
}
