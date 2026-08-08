#pragma once

#include <graphics/material.hpp>
#include <world/transform.hpp>
#include <graphics/mesh.hpp>

#include <memory>

struct GameObject {
    Transform ObjectTransform{};
    std::shared_ptr<Mesh> ObjectMesh;
    std::shared_ptr<Material> ObjectMaterial;
};
