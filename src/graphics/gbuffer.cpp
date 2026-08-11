#include <graphics/gbuffer.hpp>

#include <core/logging.hpp>

#include <glad/gl.h>

GBuffer::GBuffer(uint32_t width, uint32_t height)
    : m_width(width), m_height(height) {
    invalidate();
}

GBuffer::~GBuffer() {
    destroy();
}

void GBuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_width, m_height);
}

void GBuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::Resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return;
    if (width == m_width && height == m_height) return;

    m_width = width;
    m_height = height;

    invalidate();
}

void GBuffer::invalidate() {
    destroy();

    // FRAMEBUFFER
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    // POSITION
    m_position = std::make_unique<Texture>(
        m_width,
        m_height,
        GL_RGB16F,
        GL_RGB,
        GL_FLOAT
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_position->GetID(),
        0
    );

    // NORMAL + ROUGHNESS
    m_normalRoughness = std::make_unique<Texture>(
        m_width,
        m_height,
        GL_RGBA16F,
        GL_RGBA,
        GL_FLOAT
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT1,
        GL_TEXTURE_2D,
        m_normalRoughness->GetID(),
        0
    );

    // ALBEDO + METALLIC
    m_albedoMetallic = std::make_unique<Texture>(
        m_width,
        m_height,
        GL_SRGB8_ALPHA8,
        GL_RGBA,
        GL_UNSIGNED_BYTE
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT2,
        GL_TEXTURE_2D,
        m_albedoMetallic->GetID(),
        0
    );

    // EMISSIVE + AO
    m_emissiveAO = std::make_unique<Texture>(
        m_width,
        m_height,
        GL_RGBA16F,
        GL_RGBA,
        GL_FLOAT
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT3,
        GL_TEXTURE_2D,
        m_emissiveAO->GetID(),
        0
    );

    // DEPTH
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT24,
        m_width,
        m_height,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        m_depthTexture,
        0
    );

    // DRAW BUFFERS
    constexpr GLenum attachments[] {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };

    glDrawBuffers(4, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LogError("GBuffer framebuffer is incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::destroy() {
    m_position.reset();
    m_normalRoughness.reset();
    m_albedoMetallic.reset();
    m_emissiveAO.reset();

    if (m_depthTexture) {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }

    if (m_framebuffer) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
}
