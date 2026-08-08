#pragma once

#include <graphics/model.hpp>

#include <filesystem>
#include <memory>

class ModelLoader {
    public:
        static std::shared_ptr<Model> Load(
            const std::filesystem::path& path,
            std::shared_ptr<Material> fallbackMaterial
        );
};
