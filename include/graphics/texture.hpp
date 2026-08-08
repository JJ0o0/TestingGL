#pragma once

#include <filesystem>
#include <cstdint>

enum class TextureFormat {
    RGBA8,
    SRGBA8
};

class Texture {
    public:
        Texture(const std::filesystem::path& path, TextureFormat format = TextureFormat::SRGBA8);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        void Bind(int unit = 0) const;
        void Unbind() const;
    private:
        uint32_t m_id = 0;
};
