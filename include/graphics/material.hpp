#pragma once

#include <platform/default_resources.hpp>
#include <graphics/texture.hpp>
#include <graphics/shader.hpp>
#include <graphics/color.hpp>
#include <memory>

struct Material {
    Color Tint{1.0f};

    std::shared_ptr<Shader> MaterialShader;
    std::shared_ptr<Texture> Diffuse;

    void Apply() {
        MaterialShader->Bind();
        MaterialShader->SetColor("uMaterial.Tint", Tint);
        MaterialShader->SetInt("uMaterial.Diffuse", 0);

        if (Diffuse) Diffuse->Bind(0);
        else DefaultResources::WhiteTexture()->Bind(0);
    }
};
