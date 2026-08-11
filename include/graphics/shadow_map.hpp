#pragma once

#include <cstdint>

class ShadowMap {
    public:
        ShadowMap(uint32_t width, uint32_t height);
        ~ShadowMap();

        ShadowMap(const ShadowMap&) = delete;
        ShadowMap& operator=(const ShadowMap&) = delete;

        void Bind() const;
        static void Unbind();

        void BindTexture(uint32_t unit) const;

        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
    private:
        uint32_t m_framebuffer = 0;
        uint32_t m_depthTexture = 0;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
};
