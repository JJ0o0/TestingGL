#include <core/game.hpp>

#include <core/logging.hpp>
#include <graphics/vertex.hpp>

#include <glad/gl.h>

#include <cstdint>
#include <vector>

void Game::Initialize() {
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);

    LogInfo("GPU: {} - {}", vendor, renderer);
    LogInfo("OpenGL: {}", version);

    m_basicShader = std::make_unique<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    std::vector<Vertex> vertices {
        Vertex {
            .Position = {0.5f, 0.5f, 0.0f}
        },
        Vertex {
            .Position = {0.5f, -0.5f, 0.0f}
        },
        Vertex {
            .Position = {-0.5f, 0.5f, 0.0f}
        },
        Vertex {
            .Position = {-0.5f, -0.5f, 0.0f}
        },
    };

    std::vector<uint32_t> indices {
        0, 1, 2,
        1, 2, 3
    };

    m_quadMesh = std::make_unique<Mesh>(vertices, indices);
}

void Game::Update(float deltatime) {

}

void Game::Render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_basicShader->Bind();
    m_quadMesh->Draw();
}

void Game::Destroy() {
    m_quadMesh.reset();
    m_basicShader.reset();
}
