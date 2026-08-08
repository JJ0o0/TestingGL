#include <core/application.hpp>
#include <core/logging.hpp>
#include <core/input.hpp>
#include <platform/default_resources.hpp>

#include <glad/gl.h>

int Application::Run() {
    try {
        m_window = std::make_unique<Window>();

        Input::Initialize(m_window->GetHandle());
        DefaultResources::Initialize();

        m_game.emplace(*m_window);
        m_game->Initialize();

        float lastTime = glfwGetTime();
        float deltaTime = 0.0f;
        while (m_game->IsRunning() && !m_window->ShouldClose()) {
            Input::BeginFrame();
            m_window->PollEvents();

            float currentTime = glfwGetTime();
            deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            m_game->Update(deltaTime);
            m_game->Render();

            m_window->SwapBuffers();
        }

        stop();
        return 0;
    } catch (const std::exception& e) {
        LogError("{}", e.what());

        stop();
        return 1;
    }
}

void Application::stop() {
    if (m_game.has_value()) m_game->Destroy();

    DefaultResources::Shutdown();
    Input::Shutdown();

    m_window.reset();
}
