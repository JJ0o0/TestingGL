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
    m_material = std::make_shared<Material>(Material{
        .Tint = Color{1.0f},
        .MaterialShader = m_shader,
        .Diffuse = std::make_shared<Texture>("assets/textures/grid.png")
    });

    m_cube = std::make_unique<GameObject>(GameObject{
        .ObjectMesh = CreateCube(1.0f),
        .ObjectMaterial = m_material,
    });

    m_camera.SetPosition({0.0f, 0.0f, 6.0f});

    Input::SetKeyCallback([this](int key, InputAction action) {
        if (action == InputAction::Pressed) {
            switch (key) {
                case GLFW_KEY_F8:
                    Quit();
                    break;
            }
        }
    });
}

void Game::Update(float deltatime) {
    float rotationSpeed = 50.0f;
    if (Input::IsKeyDown(GLFW_KEY_SPACE)) rotationSpeed = 100.0f;

    m_cube->ObjectTransform.Rotate(glm::vec3{rotationSpeed * deltatime});
}

void Game::Render() {
    glClearColor(m_clearColor.R, m_clearColor.G, m_clearColor.B, m_clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawObject(*m_cube);
}

void Game::Destroy() {
    m_cube.reset();
    m_material.reset();
    m_shader.reset();
}

void Game::drawObject(const GameObject& obj) {
    obj.ObjectMaterial->Apply();

    auto& shader = obj.ObjectMaterial->MaterialShader;
    shader->SetMat4("uModel", obj.ObjectTransform.GetModelMatrix());
    shader->SetMat4("uView", m_camera.GetView());
    shader->SetMat4("uProjection", m_camera.GetProjection(m_window.GetAspectRatio()));

    obj.ObjectMesh->Draw();
}
