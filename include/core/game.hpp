#pragma once

#include <platform/window.hpp>

#include <graphics/shader.hpp>
#include <graphics/mesh.hpp>
#include <graphics/color.hpp>
#include <graphics/camera.hpp>
#include <graphics/texture.hpp>
#include <graphics/material.hpp>
#include <graphics/light.hpp>
#include <graphics/model.hpp>

#include <world/gameobject.hpp>
#include <world/transform.hpp>
#include <world/scene.hpp>

#include <utils/uuid.hpp>

#include <memory>

class Game {
    public:
        Game(Window& window) : m_window(window) {}

        void Initialize();
        void Update(float deltatime);
        void Render();
        void Destroy();

        void Quit() { m_running = false; }
        bool IsRunning() const { return m_running; }
    private:
        Window& m_window;

        bool m_running = true;

        Color m_clearColor{0.1f, 0.1f, 0.15f};

        float m_cameraYaw = 0.0f;
        float m_cameraPitch = 20.0f;
        float m_cameraDistance = 6.0f;
        float m_targetCameraDistance = 6.0f;
        Camera m_camera{};

        AmbientLight m_ambient{};
        DirectionalLight m_sun{};

        std::shared_ptr<Shader> m_shader;
        std::shared_ptr<Model> m_model;

        Scene m_scene;
        UUID m_cubeUUID;

        void showInfo();
        void initResources();
        void updateCamera(float deltatime);

        void drawObject(const GameObject& obj);
        void drawModelNode(const Model& model, uint32_t nodeIndex, const glm::mat4& parentTransform);
};
