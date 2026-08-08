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
    std::shared_ptr<Texture> Specular;

    float Shininess = 32.0f;

    void Apply() {
        MaterialShader->Bind();
        MaterialShader->SetColor("uMaterial.Tint", Tint);
        MaterialShader->SetInt("uMaterial.Diffuse", 0);
        MaterialShader->SetInt("uMaterial.Specular", 1);
        MaterialShader->SetFloat("uMaterial.Shininess", Shininess);

        if (Diffuse) Diffuse->Bind(0);
        else DefaultResources::WhiteTexture()->Bind(0);

        if (Specular) Specular->Bind(1);
        else DefaultResources::BlackTexture()->Bind(1);
    }
};
