#include <core/game.hpp>

#include <core/logging.hpp>
#include <core/input.hpp>

#include <graphics/vertex.hpp>
#include <graphics/color.hpp>
#include <graphics/premade_meshes/cube.hpp>

#include <glad/gl.h>

void Game::Initialize() {
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);

    LogInfo("GPU: {} - {}", vendor, renderer);
    LogInfo("OpenGL: {}", version);

    m_shader = std::make_shared<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    m_texture = std::make_shared<Texture>("assets/textures/grid.png");
    m_material = std::make_shared<Material>(Material{
        .Tint = Color{1.0f},
        .MaterialShader = m_shader,
        .Diffuse = m_texture
    });

    m_mesh = CreateCube(1.0f);
    m_camera.SetPosition({0.0f, 0.0f, 6.0f});
}

void Game::Update(float deltatime) {
    float rotationSpeed = 50.0f;
    if (Input::IsKeyDown(GLFW_KEY_SPACE)) rotationSpeed = 100.0f;

    m_quadTransform.Rotate(glm::vec3{rotationSpeed * deltatime});
}

void Game::Render() {
    glClearColor(m_clearColor.R, m_clearColor.G, m_clearColor.B, m_clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->Bind();

    m_shader->SetMat4("uModel", m_quadTransform.GetModelMatrix());
    m_shader->SetMat4("uView", m_camera.GetView());
    m_shader->SetMat4("uProjection", m_camera.GetProjection(m_window.GetAspectRatio()));

    m_material->Apply();
    m_mesh->Draw();
}

void Game::Destroy() {
    m_mesh.reset();
    m_material.reset();
    m_texture.reset();
    m_shader.reset();
}
