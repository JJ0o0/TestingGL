#include <core/game.hpp>

#include <core/logging.hpp>
#include <graphics/vertex.hpp>
#include <graphics/color.hpp>

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
    m_texture = std::make_unique<Texture>("assets/textures/grid.png");

    std::vector<Vertex> vertices {
        Vertex {
            .Position = {0.5f, 0.5f, 0.0f},
            .TexCoords = {1.0f, 1.0f}
        },
        Vertex {
            .Position = {0.5f, -0.5f, 0.0f},
            .TexCoords = {1.0f, 0.0f}
        },
        Vertex {
            .Position = {-0.5f, 0.5f, 0.0f},
            .TexCoords = {0.0f, 1.0f}
        },
        Vertex {
            .Position = {-0.5f, -0.5f, 0.0f},
            .TexCoords = {0.0f, 0.0f}
        },
    };

    std::vector<uint32_t> indices {
        0, 2, 1,
        1, 2, 3
    };

    m_quadMesh = std::make_unique<Mesh>(vertices, indices);
    m_camera.SetPosition({0.0f, 0.0f, 6.0f});
}

void Game::Update(float deltatime) {
    const float rotationSpeed = 40.0f;
    float yRot = m_quadTransform.GetEulerRotation().y;
    m_quadTransform.RotateEuler({
        0.0f, yRot + (rotationSpeed * deltatime), 0.0f
    });
}

void Game::Render() {
    glClearColor(m_clearColor.R, m_clearColor.G, m_clearColor.B, m_clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_basicShader->Bind();

    m_basicShader->SetMat4("uModel", m_quadTransform.GetModelMatrix());
    m_basicShader->SetMat4("uView", m_camera.GetView());
    m_basicShader->SetMat4("uProjection", m_camera.GetProjection(m_window.GetAspectRatio()));

    m_basicShader->SetInt("uTexture", 0);
    m_texture->Bind();
        m_quadMesh->Draw();
    m_texture->Unbind();
}

void Game::Destroy() {
    m_quadMesh.reset();
    m_texture.reset();
    m_basicShader.reset();
}
