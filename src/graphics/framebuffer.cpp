#include <graphics/framebuffer.hpp>

#include <core/logging.hpp>

#include <glad/gl.h>

Framebuffer::Framebuffer(const FramebufferProperties& properties)
    : m_properties(properties){
    invalidate();
}

Framebuffer::~Framebuffer() {
    destroy();
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glViewport(0, 0, m_properties.Width, m_properties.Height);
}

void Framebuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0 || (width == m_properties.Width && height == m_properties.Height)) return;

    m_properties.Width = width;
    m_properties.Height = height;

    destroy();
    invalidate();
}

void Framebuffer::invalidate() {
    glGenFramebuffers(1, &m_id);
    glBindFramebuffer(GL_FRAMEBUFFER,m_id);

    // COLOR ATTACHMENT
    m_colorAttachment = std::make_unique<Texture>(
        m_properties.Width, m_properties.Height,
        m_properties.ColorInternalFormat,
        m_properties.ColorFormat,
        m_properties.ColorType,
        m_properties.Filter
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_colorAttachment->GetID(),
        0
    );

    // DEPTH
    if (m_properties.HasDepth) {
        glGenRenderbuffers(1, &m_depthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);

        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH_COMPONENT24,
            m_properties.Width, m_properties.Height
        );

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            m_depthRBO
        );
    }

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
