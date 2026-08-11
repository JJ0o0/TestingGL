#pragma once

#include <cstdint>
#include <memory>

#include <graphics/texture.hpp>

class GBuffer {
    public:
        GBuffer(uint32_t width, uint32_t height);
        ~GBuffer();

        GBuffer(const GBuffer&) = delete;
        GBuffer& operator=(const GBuffer&) = delete;

        void Bind() const;
        static void Unbind();

        void Resize(uint32_t width, uint32_t height);

        uint32_t GetID() const { return m_framebuffer; }
        uint32_t GetWidth() const { return m_width;}
        uint32_t GetHeight() const { return m_height;}

        const Texture& GetPosition() const { return *m_position; }
        const Texture& GetNormalRoughness() const { return *m_normalRoughness; }
        const Texture& GetAlbedoMetallic() const { return *m_albedoMetallic; }
        const Texture& GetEmissiveAO() const { return *m_emissiveAO; }
    private:
        uint32_t m_framebuffer = 0;
        uint32_t m_depthTexture = 0;

        uint32_t m_width = 0;
        uint32_t m_height = 0;

        std::unique_ptr<Texture> m_position;
        std::unique_ptr<Texture> m_normalRoughness;
        std::unique_ptr<Texture> m_albedoMetallic;
        std::unique_ptr<Texture> m_emissiveAO;

        void invalidate();
        void destroy();
};
