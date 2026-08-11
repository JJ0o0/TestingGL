#include <graphics/material.hpp>

#include <core/logging.hpp>
#include <platform/default_resources.hpp>

#include <algorithm>

void Material::Apply(Shader& shader) const {
    shader.SetColor("uMaterial.BaseColor", BaseColor);
    shader.SetInt("uBaseColorTexture", 0);

    if (BaseColorTexture) BaseColorTexture->Bind(0);
    else DefaultResources::WhiteTexture()->Bind(0);

    shader.SetInt("uMaterial.AlphaMode", static_cast<int>(Alpha));
    shader.SetFloat("uMaterial.AlphaCutoff", AlphaCutoff);

    if (Type != MaterialType::PBR) return;

    shader.SetColor("uMaterial.EmissiveColor", EmissiveColor);

    shader.SetFloat("uMaterial.Metallic", std::clamp(Metallic, 0.0f, 1.0f));
    shader.SetFloat("uMaterial.Roughness", std::clamp(Roughness, 0.0f, 1.0f));
    shader.SetFloat("uMaterial.NormalScale", NormalScale);
    shader.SetFloat("uMaterial.EmissiveStrength", EmissiveStrength);
    shader.SetFloat("uMaterial.OcclusionStrength", OcclusionStrength);

    shader.SetInt("uARMTexture", 1);
    shader.SetInt("uNormalTexture", 2);
    shader.SetInt("uEmissiveTexture", 3);

    if (ARMTexture) ARMTexture->Bind(1);
    else DefaultResources::WhiteTexture()->Bind(1);

    if (NormalTexture) NormalTexture->Bind(2);
    else DefaultResources::FlatNormalTexture()->Bind(2);

    if (EmissiveTexture) EmissiveTexture->Bind(3);
    else DefaultResources::WhiteTexture()->Bind(3);
}
