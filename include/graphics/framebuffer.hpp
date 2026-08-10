#pragma once

#include <graphics/texture.hpp>

#include <cstdint>
#include <memory>

class Framebuffer {
    public:
        Framebuffer(uint32_t width, uint32_t height);
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        void Bind() const;
        static void Unbind();

        void Resize(uint32_t width, uint32_t height);

        const Texture& GetColorAttachment() const { return *m_colorAttachment; }

        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
    public:
        uint32_t m_id = 0;
        uint32_t m_depthRBO = 0;

        uint32_t m_width = 0;
        uint32_t m_height = 0;

        std::unique_ptr<Texture> m_colorAttachment;

        void invalidate();
        void destroy();
};
