#pragma once

#include <graphics/texture.hpp>

#include <glad/gl.h>

#include <cstdint>
#include <memory>

struct FramebufferProperties {
    uint32_t Width;
    uint32_t Height;

    GLenum ColorInternalFormat = GL_RGBA16F;
    GLenum ColorFormat = GL_RGBA;
    GLenum ColorType = GL_FLOAT;
    GLenum Filter = GL_LINEAR;

    bool HasDepth = true;
};

class Framebuffer {
    public:
        Framebuffer(const FramebufferProperties& properties);
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        void Bind() const;
        static void Unbind();

        void Resize(uint32_t width, uint32_t height);

        const Texture& GetColorAttachment() const { return *m_colorAttachment; }

        uint32_t GetID() const { return m_id; }
        uint32_t GetWidth() const { return m_properties.Width; }
        uint32_t GetHeight() const { return m_properties.Height; }
    private:
        uint32_t m_id = 0;
        uint32_t m_depthRBO = 0;

        FramebufferProperties m_properties;

        std::unique_ptr<Texture> m_colorAttachment;

        void invalidate();
        void destroy();
};
