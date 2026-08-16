#pragma once

#include <graphics/reflection_probe.hpp>
#include <graphics/environment.hpp>
#include <graphics/light.hpp>

#include <world/gameobject.hpp>

#include <utils/uuid.hpp>

#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include <memory>

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

        void AddPointLight(const PointLight& light);
        const std::vector<PointLight>& GetPointLights() const { return m_pointLights; }

        void AddSpotLight(const SpotLight& light);
        const std::vector<SpotLight>& GetSpotLights() const { return m_spotLights; }

        ReflectionProbe& AddReflectionProbe(ReflectionProbe probe);
        std::vector<ReflectionProbe>& GetReflectionProbes() { return m_reflectionProbes; }
        const std::vector<ReflectionProbe>& GetReflectionProbes() const { return m_reflectionProbes; }

        void SetEnvironment(std::shared_ptr<Environment> environment) { m_environment = std::move(environment);}
        Environment* GetEnvironment() { return m_environment.get(); }
        const Environment* GetEnvironment() const { return m_environment.get(); }

        void Clear();
    private:
        std::unordered_map<UUID, GameObject> m_gameObjects;

        std::shared_ptr<Environment> m_environment;

        AmbientLight m_ambientLight{};
        DirectionalLight m_sun{};
        std::vector<PointLight> m_pointLights;
        std::vector<SpotLight> m_spotLights;

        std::vector<ReflectionProbe> m_reflectionProbes;
};
