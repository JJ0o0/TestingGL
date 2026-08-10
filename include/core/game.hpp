#pragma once

#include <core/renderer.hpp>

#include <platform/window.hpp>

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

        float m_cameraYaw = 0.0f;
        float m_cameraPitch = 20.0f;
        float m_cameraDistance = 6.0f;
        float m_targetCameraDistance = 6.0f;
        Camera m_camera{};

        std::shared_ptr<Shader> m_shader;
        std::shared_ptr<Cubemap> m_environmentMap;
        std::shared_ptr<Cubemap> m_irradianceMap;
        std::shared_ptr<Cubemap> m_prefilterMap;
        std::shared_ptr<Texture> m_brdfLUT;
        std::shared_ptr<Model> m_model;

        Scene m_scene;
        UUID m_modelUUID;

        void showInfo();
        void initResources();
        void updateCamera(float deltatime);
};
