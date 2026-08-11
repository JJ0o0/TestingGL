#pragma once

#include <graphics/texture.hpp>
#include <graphics/shader.hpp>
#include <graphics/color.hpp>

#include <memory>

enum class MaterialType {
    PBR,
    Unlit
};

enum class AlphaMode {
    Opaque,
    Mask,
    Blend
};

struct Material {
    MaterialType Type = MaterialType::PBR;

    AlphaMode Alpha = AlphaMode::Opaque;
    float AlphaCutoff = 0.5f;

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

    void ApplyBase(Shader& shader) const;
    void ApplyPBR(Shader& shader) const;
};
