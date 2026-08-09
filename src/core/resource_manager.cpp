#include <core/resource_manager.hpp>

std::unordered_map<std::string, std::shared_ptr<Texture>> ResourceManager::s_textures;

std::shared_ptr<Texture> ResourceManager::GetTexture(const std::string& key) {
    const auto it = s_textures.find(key);
    if (it == s_textures.end()) return nullptr;

    return it->second;
}

bool ResourceManager::HasTexture(const std::string &key) { return s_textures.contains(key); }
bool ResourceManager::RemoveTexture(const std::string &key) { return s_textures.erase(key) > 0; }
size_t ResourceManager::GetTextureCount() { return s_textures.size(); }

void ResourceManager::Clear() {
    s_textures.clear();
}
