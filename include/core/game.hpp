#pragma once

#include <platform/window.hpp>
#include <graphics/shader.hpp>
#include <graphics/mesh.hpp>
#include <graphics/color.hpp>
#include <graphics/camera.hpp>
#include <graphics/texture.hpp>
#include <graphics/material.hpp>
#include <world/gameobject.hpp>
#include <world/transform.hpp>
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
        Camera m_camera{};

        std::shared_ptr<Shader> m_shader;
        std::shared_ptr<Material> m_material;

        std::unique_ptr<GameObject> m_cube;

        void drawObject(const GameObject& obj);
};
