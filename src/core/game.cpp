#include <core/game.hpp>

#include <core/logging.hpp>
#include <core/input.hpp>

#include <graphics/vertex.hpp>
#include <graphics/color.hpp>
#include <graphics/model_loader.hpp>
#include <graphics/premade_meshes/cube.hpp>

#include <glad/gl.h>

void Game::Initialize() {
    showInfo();

    Input::SetKeyCallback([this](int key, InputAction action) {
        if (action == InputAction::Pressed) {
            switch (key) {
                case GLFW_KEY_F8:
                    Quit();
                    break;
            }
        }
    });

    initResources();

    m_camera.SetPosition({0.0f, 0.0f, 6.0f});
    m_sun.Direction = {-0.3f, -1.0f, -0.4f};
}

void Game::Update(float deltatime) {
    updateCamera(deltatime);
}

void Game::Render() {
    m_renderer.Render(
        m_scene,
        m_camera,
        m_ambient, m_sun,
        m_clearColor
    );
}

void Game::Destroy() {
    m_scene.Clear();

    m_model.reset();
    m_shader.reset();
}

void Game::showInfo() {
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);

    LogInfo("GPU: {} - {}", vendor, renderer);
    LogInfo("OpenGL: {}", version);
}

void Game::initResources() {
    m_shader = std::make_shared<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    auto material = std::make_shared<Material>(Material{
        .Tint = Color{1.0f},
        .MaterialShader = m_shader,
        .Diffuse = std::make_shared<Texture>("assets/textures/grid.png", TextureFormat::SRGBA8),
    });

    m_model = ModelLoader::Load("assets/models/survival_guitar_backpack.glb", material);
    if (!m_model) {
        LogError("Failed to load test model");
        return;
    }

    GameObject& cube = m_scene.CreateGameObject("Cube");
    cube.SetModel(m_model);
    cube.GetTransform().Scale *= 0.005f;

    m_cubeUUID = cube.GetUUID();
}

void Game::updateCamera(float deltatime) {
    const glm::vec2 mouseDelta = Input::GetMouseDelta();
    const float sensitivity = 0.3f;

    if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT)) {
        m_cameraYaw -= mouseDelta.x * sensitivity;
        m_cameraPitch -= mouseDelta.y * sensitivity;

        Input::SetCursorMode(CursorMode::Disabled);
    } else if (Input::IsMouseButtonReleased(GLFW_MOUSE_BUTTON_RIGHT)) {
        Input::SetCursorMode(CursorMode::Normal);
    }

    m_cameraPitch = glm::clamp(m_cameraPitch, -89.0f, 89.0f);

    m_targetCameraDistance -= Input::GetScrollDelta() * 0.5f;
    m_targetCameraDistance = glm::clamp(m_targetCameraDistance, 2.0f, 20.0f);

    constexpr float zoomSmoothness = 8.0f;
    float zoomT = 1.0f - std::exp(-zoomSmoothness * deltatime);

    m_cameraDistance = glm::mix(m_cameraDistance, m_targetCameraDistance, zoomT);

    const float yaw = glm::radians(m_cameraYaw);
    const float pitch = glm::radians(m_cameraPitch);

    const GameObject* targetObj = m_scene.GetGameObject(m_cubeUUID);
    if (!targetObj) return;

    const glm::vec3 target = targetObj->GetTransform().Position;

    glm::vec3 offset {
        m_cameraDistance * glm::cos(pitch) * glm::sin(yaw),
        m_cameraDistance * glm::sin(pitch),
        m_cameraDistance * glm::cos(pitch) * glm::cos(yaw)
    };

    glm::vec3 targetPosition = target + offset;

    float smoothness = 12.0f;
    float t = 1.0f - std::exp(-smoothness * deltatime);

    glm::vec3 newPosition = glm::mix(m_camera.GetPosition(), targetPosition, t);

    m_camera.SetPosition(newPosition);
    m_camera.LookAt(target);
}
