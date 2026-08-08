#pragma once

#include <graphics/shader.hpp>
#include <graphics/mesh.hpp>
#include <graphics/color.hpp>
#include <math/transform.hpp>
#include <memory>

class Game {
    public:
        void Initialize();
        void Update(float deltatime);
        void Render();
        void Destroy();

        void Quit() { m_running = false; }
        bool IsRunning() const { return m_running; }
    private:
        bool m_running = true;

        Color m_clearColor{0.1f, 0.1f, 0.15f};
        Transform m_quadTransform{};

        std::unique_ptr<Shader> m_basicShader;
        std::unique_ptr<Mesh> m_quadMesh;
};
