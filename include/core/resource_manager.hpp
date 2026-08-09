#pragma once

#include <graphics/texture.hpp>

#include <unordered_map>
#include <utility>
#include <string>
#include <memory>

class ResourceManager {
    public:
        static std::shared_ptr<Texture> GetTexture(const std::string& key);

        template<typename Factory>
        static std::shared_ptr<Texture> GetOrCreateTexture(std::string key, Factory&& factory) {
            if (auto it = s_textures.find(key); it != s_textures.end()) return it->second;

            auto texture = std::forward<Factory>(factory)();
            if (!texture) return nullptr;

            s_textures.emplace(std::move(key), texture);
            return texture;
        }

        static bool HasTexture(const std::string& key);
        static bool RemoveTexture(const std::string& key);

        static size_t GetTextureCount();

        static void Clear();
    private:
        static std::unordered_map<std::string, std::shared_ptr<Texture>> s_textures;
};
