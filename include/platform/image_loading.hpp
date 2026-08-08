#pragma once

#include <core/error_handling.hpp>
#include <stb/stb_image.h>
#include <filesystem>
#include <cstdint>
#include <vector>
#include <memory>

struct ImageData {
    std::vector<uint8_t> Pixels;

    uint32_t Width = 0;
    uint32_t Height = 0;
};

inline ImageData LoadImage(const std::filesystem::path& path) {
    const std::string pathStr = path.string();
    int width = 0, height = 0;

    using STBIData = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
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
