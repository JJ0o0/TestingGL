#pragma once

#include <core/renderer.hpp>

#include <platform/window.hpp>

#include <graphics/environment.hpp>
#include <graphics/material.hpp>
#include <graphics/texture.hpp>
#include <graphics/cubemap.hpp>
#include <graphics/camera.hpp>
#include <graphics/shader.hpp>
#include <graphics/color.hpp>
#include <graphics/light.hpp>
#include <graphics/model.hpp>
#include <graphics/mesh.hpp>

#include <world/gameobject.hpp>
#include <world/transform.hpp>
#include <world/scene.hpp>

#include <utils/uuid.hpp>

#include <memory>

class Game {
    public:
        Game(Window& window)
            : m_window(window), m_renderer(window) {}

        void Initialize();
        void Update(float deltatime);
        void Render();
        void Destroy();

        void Quit() { m_running = false; }
        bool IsRunning() const { return m_running; }
    private:
        Window& m_window;
        Renderer m_renderer;

        bool m_running = true;

        Color m_clearColor{0.1f, 0.1f, 0.15f};

        glm::vec3 m_cameraVelocity{0.0f};
        float m_cameraYaw = -90.0f;
        float m_cameraPitch = 0.0f;
        Camera m_camera{};

        Scene m_scene;

        void showInfo();
        void initResources();
        void updateCamera(float deltatime);
};
