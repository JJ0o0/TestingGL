#include <world/scene.hpp>

GameObject& Scene::CreateGameObject(std::string name) {
    GameObject obj(std::move(name));
    const UUID uuid = obj.GetUUID();

    auto [it, inserted] = m_gameObjects.emplace(uuid, std::move(obj));
    return it->second;
}

GameObject* Scene::GetGameObject(const UUID& uuid) {
    const auto it = m_gameObjects.find(uuid);
    if (it == m_gameObjects.end()) return nullptr;

    return &it->second;
}

const GameObject* Scene::GetGameObject(const UUID& uuid) const {
    const auto it = m_gameObjects.find(uuid);
    if (it == m_gameObjects.end()) return nullptr;

    return &it->second;
}

bool Scene::DestroyGameObject(const UUID& uuid) {
    return m_gameObjects.erase(uuid) > 0;
}

void Scene::Clear() {
    m_gameObjects.clear();
}
