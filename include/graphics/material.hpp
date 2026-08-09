#pragma once

#include <graphics/texture.hpp>
#include <graphics/shader.hpp>
#include <graphics/color.hpp>

#include <memory>

struct Material {
    Color BaseColor{1.0f};
    Color EmissiveColor{0.0f, 0.0f, 0.0f, 1.0f};

    float Metallic = 1.0f;
    float Roughness = 1.0f;
    float NormalScale = 1.0f;
    float EmissiveStrength = 1.0f;
    float OcclusionStrength = 1.0f;

    std::shared_ptr<Texture> BaseColorTexture;
    std::shared_ptr<Texture> ARMTexture;
    std::shared_ptr<Texture> NormalTexture;
    std::shared_ptr<Texture> EmissiveTexture;

    std::shared_ptr<Shader> MaterialShader;

    void Apply() const;
};
