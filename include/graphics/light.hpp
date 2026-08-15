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

    glm::vec3 Direction{-0.3f, -1.0f, -0.4f};

    void Apply(Shader& shader) const {
        shader.Bind();
        shader.SetFloat("uDirectionalLight.Intensity", Intensity);
        shader.SetColor3("uDirectionalLight.Tint", Tint);
        shader.SetVec3("uDirectionalLight.Direction", Direction);
    }
};

struct PointLight {
    float Intensity = 1.0f;
    Color Tint{1.0f};

    glm::vec3 Position{0.0f};
    float Radius = 5.0f;
};

struct SpotLight {
    float Intensity = 1.0f;
    Color Tint{1.0f};

    glm::vec3 Position{0.0f};
    glm::vec3 Direction{-0.3f, -1.0f, -0.4f};

    float Radius = 10.0f;

    float InnerCone = 20.0f;
    float OuterCone = 30.0f;
};
