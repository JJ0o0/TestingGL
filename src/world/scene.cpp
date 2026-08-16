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

void Scene::AddPointLight(const PointLight& light) {
    m_pointLights.push_back(light);
}

void Scene::AddSpotLight(const SpotLight& light) {
    m_spotLights.push_back(light);
}

ReflectionProbe& Scene::AddReflectionProbe(ReflectionProbe probe) {
    m_reflectionProbes.push_back(std::move(probe));
    return m_reflectionProbes.back();
}

void Scene::InvalidateReflectionProbes() {
    ++m_captureRevision;
}

void Scene::Clear() {
    m_environment.reset();
    m_reflectionProbes.clear();
    m_spotLights.clear();
    m_pointLights.clear();
    m_gameObjects.clear();
}
