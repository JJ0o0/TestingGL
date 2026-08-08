#pragma once

#include <string_view>
#include <algorithm>
#include <optional>
#include <cstdint>
#include <cmath>

// 0.0 - 1.0
struct Color {
    float R = 1.0f;
    float G = 1.0f;
    float B = 1.0f;
    float A = 1.0f;

    constexpr Color() = default;
    constexpr Color(float r, float g, float b) : R(r), G(g), B(b), A(1.0f) {}
    constexpr Color(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}
    explicit constexpr Color(float value) : R(value), G(value), B(value), A(value) {}
};

// 0 - 255
struct Color32 {
    uint8_t R = 255;
    uint8_t G = 255;
    uint8_t B = 255;
    uint8_t A = 255;

    constexpr Color32() = default;
    constexpr Color32(uint8_t r, uint8_t g, uint8_t b) : R(r), G(g), B(b), A(255) {}
    constexpr Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a) {}
    explicit constexpr Color32(uint8_t value) : R(value), G(value), B(value), A(value) {}

    bool operator==(const Color32&) const = default;
};

// Comparar cores
inline bool ApproximatelyEqual(const Color& a, const Color& b, float epsilon = 0.00001f) {
    return
        std::abs(a.R - b.R) <= epsilon &&
        std::abs(a.G - b.G) <= epsilon &&
        std::abs(a.B - b.B) <= epsilon &&
        std::abs(a.A - b.A) <= epsilon;
}

// Converte de SRGB para Linear
inline float SrgbToLinear(float channel) noexcept {
    if (channel <= 0.04045f) return channel / 12.92f;

    return std::pow((channel + 0.055f) / 1.055f, 2.4f);
}

// Converte os canais de 0-255 para 0.0-1.0
inline Color ToColor(const Color32& color32) {
    const auto toLinear = [](std::uint8_t channel) {
        const float srgb = static_cast<float>(channel) / 255.0f;
        return SrgbToLinear(srgb);
    };

    return Color {
        toLinear(color32.R),
        toLinear(color32.G),
        toLinear(color32.B),
        static_cast<float>(color32.A) / 255.0f,
    };
}

// Converte os canais de 0.0-1.0 para 0-255
inline Color32 ToColor32(const Color& color) {
    const auto toByte = [](float channel) constexpr -> uint8_t {
        channel = std::clamp(channel, 0.0f, 1.0f);
        return static_cast<uint8_t>((channel * 255.0f) + 0.5f);
    };

    return Color32 {
        toByte(color.R),
        toByte(color.G),
        toByte(color.B),
        toByte(color.A),
    };
}

namespace detail {
    // Converte um caractere hexadecimal em número
    inline int HexDigitValue(char character) {
        if (character >= '0' && character <= '9') return character - '0';
        if (character >= 'A' && character <= 'F') return character - 'A' + 10;
        if (character >= 'a' && character <= 'f') return character - 'a' + 10;

        return -1;
    }

    // Recebe dois dígitos hexadecimais e forma um byte
    inline std::optional<uint8_t> ParseHexByte(char first, char second) {
        const int high = HexDigitValue(first);
        const int low = HexDigitValue(second);

        if (high < 0 || low < 0) return std::nullopt;
        return static_cast<uint8_t>((high * 16) + low);
    }
}

// Converte uma cor em hexadecimal para Color32 (0-255)
inline std::optional<Color32> ParseHexColor32(std::string_view hexadecimal) {
    if (!hexadecimal.empty() && hexadecimal.front() == '#') hexadecimal.remove_prefix(1);
    if (hexadecimal.size() != 6 && hexadecimal.size() != 8) return std::nullopt;

    const auto red = detail::ParseHexByte(hexadecimal[0], hexadecimal[1]);
    const auto green = detail::ParseHexByte(hexadecimal[2], hexadecimal[3]);
    const auto blue = detail::ParseHexByte(hexadecimal[4], hexadecimal[5]);

    if (!red || !green || !blue) return std::nullopt;

    uint8_t alpha = 255;
    if (hexadecimal.size() == 8) {
        const auto parsedAlpha = detail::ParseHexByte(hexadecimal[6], hexadecimal[7]);
        if (!parsedAlpha) return std::nullopt;

        alpha = *parsedAlpha;
    }

    return Color32 {
        *red,
        *green,
        *blue,
        alpha
    };
}

// Converte uma cor em hexadecimal para Color (0.0-1.0)
inline std::optional<Color> ParseHexColor(std::string_view hexadecimal) {
    const auto color32 = ParseHexColor32(hexadecimal);
    if (!color32) return std::nullopt;

    return ToColor(*color32);
}
