#pragma once

#include <graphics/texture.hpp>
#include <graphics/cubemap.hpp>

#include <cstdint>
#include <memory>

struct Environment {
    std::shared_ptr<Cubemap> Skybox;
    std::shared_ptr<Cubemap> Irradiance;
    std::shared_ptr<Cubemap> Prefilter;
};

std::shared_ptr<Environment> CreateEnvironment(
    const Texture& equirectangularHDR,
    uint32_t environmentSize = 512,
    uint32_t irradianceSize = 32,
    uint32_t prefilterSize = 128
);
