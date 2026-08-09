#pragma once

#include <graphics/shader.hpp>
#include <graphics/color.hpp>

struct AmbientLight {
    float Intensity = 0.1f;
    Color Tint{1.0f};

    void Apply(Shader& shader) const {
        shader.Bind();
        shader.SetFloat("uAmbient.Intensity", Intensity);
        shader.SetColor3("uAmbient.Tint", Tint);
    }
};

struct DirectionalLight {
    float Intensity = 1.0f;
    Color Tint{1.0f};

    glm::vec3 Direction{0.0f, -1.0f, 0.0f};

    void Apply(Shader& shader) const {
        shader.Bind();
        shader.SetFloat("uDirectionalLight.Intensity", Intensity);
        shader.SetColor3("uDirectionalLight.Tint", Tint);
        shader.SetVec3("uDirectionalLight.Direction", Direction);
    }
};
