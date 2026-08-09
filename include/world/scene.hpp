#pragma once

#include <graphics/light.hpp>
#include <world/gameobject.hpp>
#include <utils/uuid.hpp>

#include <unordered_map>
#include <string>

class Scene {
    public:
        // GAMEOBJECTS
        GameObject& CreateGameObject(std::string name = "GameObject");

        GameObject* GetGameObject(const UUID& uuid);
        const GameObject* GetGameObject(const UUID& uuid) const;

        bool DestroyGameObject(const UUID& uuid);

        const auto& GetGameObjects() const { return m_gameObjects; }

        // LIGHTING
        AmbientLight& GetAmbientLight() { return m_ambientLight; }
        const AmbientLight& GetAmbientLight() const { return m_ambientLight; }

        DirectionalLight& GetSun() { return m_sun; }
        const DirectionalLight& GetSun() const { return m_sun; }

        void Clear();
    private:
        std::unordered_map<UUID, GameObject> m_gameObjects;

        AmbientLight m_ambientLight{};
        DirectionalLight m_sun{};
};
