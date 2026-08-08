#pragma once

#include <graphics/shader.hpp>
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

        std::unique_ptr<Shader> m_basicShader;
};
