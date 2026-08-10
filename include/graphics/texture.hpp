#pragma once

#include <graphics/image_data.hpp>
#include <filesystem>
#include <cstdint>

enum class TextureFormat {
    RGBA8,
    SRGBA8
};

class Texture {
    public:
        Texture(const std::filesystem::path& path, TextureFormat format = TextureFormat::SRGBA8);
        Texture(const ImageData& image, TextureFormat format = TextureFormat::SRGBA8);
        Texture(const HDRImageData& image);
        Texture(uint32_t width, uint32_t height, int internalFormat, int format, int type);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        void Bind(int unit = 0) const;
        void Unbind() const;

        uint32_t GetID() const { return m_id; }
    private:
        uint32_t m_id = 0;
};
