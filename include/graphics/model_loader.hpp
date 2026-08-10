#pragma once

#include <core/resource_manager.hpp>
#include <graphics/model.hpp>

#include <filesystem>
#include <memory>

class ModelLoader {
    public:
        static std::shared_ptr<Model> Load(const std::filesystem::path& path);
};
