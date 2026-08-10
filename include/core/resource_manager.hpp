#pragma once

#include <core/resource_cache.hpp>

#include <graphics/texture.hpp>
#include <graphics/model.hpp>
#include <graphics/environment.hpp>

#include <filesystem>
#include <memory>

class ResourceManager {
    public:
        template<typename T, typename Factory>
        static std::shared_ptr<T> GetOrCreate(std::string key, Factory&& factory) {
            return ResourceCache<T>::GetOrCreate(std::move(key), std::forward<Factory>(factory));
        }

        static std::shared_ptr<Texture> LoadTexture(const std::filesystem::path& path);
        static std::shared_ptr<Model> LoadModel(const std::filesystem::path& path);
        static std::shared_ptr<Environment> LoadEnvironment(const std::filesystem::path& path);

        static void Clear();
};
