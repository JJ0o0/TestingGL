#pragma once

#include <world/gameobject.hpp>
#include <utils/uuid.hpp>

#include <unordered_map>
#include <string>

class Scene {
    public:
        GameObject& CreateGameObject(std::string name = "GameObject");

        GameObject* GetGameObject(const UUID& uuid);
        const GameObject* GetGameObject(const UUID& uuid) const;

        bool DestroyGameObject(const UUID& uuid);

        void Clear();

        const std::unordered_map<UUID, GameObject>& GetGameObjects() const { return m_gameObjects; }
    private:
        std::unordered_map<UUID, GameObject> m_gameObjects;
};
