#include <graphics/material.hpp>

#include <core/logging.hpp>
#include <platform/default_resources.hpp>

#include <algorithm>

void Material::Apply() const {
    if (!MaterialShader) {
        LogError("Tried to apply material without a shader!");
        return;
    }

    MaterialShader->Bind();
    MaterialShader->SetColor("uMaterial.BaseColor", BaseColor);
    MaterialShader->SetColor("uMaterial.EmissiveColor", EmissiveColor);

    MaterialShader->SetFloat("uMaterial.Metallic", std::clamp(Metallic, 0.0f, 1.0f));
    MaterialShader->SetFloat("uMaterial.Roughness", std::clamp(Roughness, 0.0f, 1.0f));
    MaterialShader->SetFloat("uMaterial.NormalScale", NormalScale);
    MaterialShader->SetFloat("uMaterial.EmissiveStrength", EmissiveStrength);
    MaterialShader->SetFloat("uMaterial.OcclusionStrength", OcclusionStrength);

    MaterialShader->SetInt("uBaseColorTexture", 0);
    MaterialShader->SetInt("uARMTexture", 1);
    MaterialShader->SetInt("uNormalTexture", 2);
    MaterialShader->SetInt("uEmissiveTexture", 3);

    if (BaseColorTexture) BaseColorTexture->Bind(0);
    else DefaultResources::WhiteTexture()->Bind(0);

    if (ARMTexture) ARMTexture->Bind(1);
    else DefaultResources::WhiteTexture()->Bind(1);

    if (NormalTexture) NormalTexture->Bind(2);
    else DefaultResources::FlatNormalTexture()->Bind(2);

    if (EmissiveTexture) EmissiveTexture->Bind(3);
    else DefaultResources::WhiteTexture()->Bind(3);
}
