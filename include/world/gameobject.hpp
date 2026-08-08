#pragma once

#include <graphics/model.hpp>
#include <world/transform.hpp>

#include <memory>

struct GameObject {
    Transform ObjectTransform{};
    std::shared_ptr<Model> ObjectModel;
};
