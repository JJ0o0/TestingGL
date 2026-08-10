#include <core/resource_manager.hpp>

#include <graphics/model_loader.hpp>
#include <graphics/environment.hpp>

#include <platform/image_loading.hpp>

std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::filesystem::path& path) {
    const std::string key = path.lexically_normal().string();

    return ResourceCache<Texture>::GetOrCreate(key, [&]() {
        return std::make_shared<Texture>(path);
    });
}

std::shared_ptr<Model> ResourceManager::LoadModel(const std::filesystem::path& path) {
    const std::string key = path.lexically_normal().string();

    return ResourceCache<Model>::GetOrCreate(key, [&]() {
        return ModelLoader::Load(path);
    });
}

std::shared_ptr<Environment>
ResourceManager::LoadEnvironment(const std::filesystem::path& path) {
    const std::string key = path.lexically_normal().string();

    return ResourceCache<Environment>::GetOrCreate(key, [&]() {
        HDRImageData hdr = LoadHDRImage(path, true);
        Texture hdrTexture(hdr);

        return CreateEnvironment(hdrTexture);
    });
}

void ResourceManager::Clear() {
    ResourceCache<Environment>::Clear();
    ResourceCache<Model>::Clear();
    ResourceCache<Texture>::Clear();
}
