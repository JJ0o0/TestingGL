#pragma once

#include <graphics/cubemap.hpp>
#include <glm/vec3.hpp>
#include <cstdint>
#include <memory>

struct ReflectionProbe {
    glm::vec3 Position{0.0f};

    glm::vec3 BoxMin{0.0f};
    glm::vec3 BoxMax{1.0f};

    std::shared_ptr<Cubemap> Irradiance;
    std::shared_ptr<Cubemap> Prefilter;

    float Intensity = 1.0f;
    float BlendDistance = 0.5f;

    uint64_t LastBakeRevision = 0;
};
