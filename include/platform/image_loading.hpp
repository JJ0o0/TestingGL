#pragma once

#include <core/error_handling.hpp>
#include <graphics/image_data.hpp>
#include <stb/stb_image.h>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <span>

inline ImageData LoadImage(const std::filesystem::path& path, bool flipVertically = false) {
    const std::string pathStr = path.string();
    int width = 0, height = 0;

    using STBIData = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

    stbi_set_flip_vertically_on_load(flipVertically);
    STBIData stbiData {
        stbi_load(pathStr.c_str(), &width, &height, nullptr, STBI_rgb_alpha),
        &stbi_image_free
    };

    CheckError(stbiData != nullptr, "Texture Loading", "Failed loading texture at {}: {}", pathStr, stbi_failure_reason());
    CheckError(width > 0 && height > 0, "Texture Loading", "Texture at {} has invalid dimensions: {}x{}", pathStr, width, height);

    ImageData result{};
    result.Width = static_cast<uint32_t>(width);
    result.Height = static_cast<uint32_t>(height);

    const size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height) * STBI_rgb_alpha;
    result.Pixels.assign(stbiData.get(), stbiData.get() + byteCount);

    return result;
}

inline ImageData LoadImage(std::span<const uint8_t> data, bool flipVertically = false) {
    CheckError(!data.empty(), "Texture Loading", "Tried loading image from empty memory buffer");

    int width = 0, height = 0;

    using STBIData = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

    stbi_set_flip_vertically_on_load(flipVertically);
    STBIData stbiData{
        stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &width, &height, nullptr, STBI_rgb_alpha),
        &stbi_image_free
    };

    CheckError(stbiData != nullptr, "Texture Loading", "Failed loading texture from memory: {}", stbi_failure_reason());
    CheckError(width > 0 && height > 0, "Texture Loading", "Texture loaded from memory has invalid dimensions: {}x{}", width, height);

    ImageData result{};
    result.Width = static_cast<uint32_t>(width);
    result.Height = static_cast<uint32_t>(height);

    const size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height) * STBI_rgb_alpha;
    result.Pixels.assign(
        stbiData.get(),
        stbiData.get() + byteCount
    );

    return result;
}
