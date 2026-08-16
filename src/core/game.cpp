#include <core/game.hpp>

#include <core/logging.hpp>
#include <core/input.hpp>

#include <platform/image_loading.hpp>

#include <graphics/vertex.hpp>
#include <graphics/color.hpp>
#include <graphics/environment.hpp>
#include <graphics/model_loader.hpp>
#include <graphics/premade_meshes/cube.hpp>

#include <glad/gl.h>

void Game::Initialize() {
    showInfo();

    Input::SetKeyCallback([this](int key, InputAction action) {
        if (action == InputAction::Pressed) {
            switch (key) {
                case GLFW_KEY_ESCAPE: {
                    const bool locked = Input::IsCursorCaptured();
                    if (locked) Input::SetCursorMode(CursorMode::Normal);
                    else Input::SetCursorMode(CursorMode::Disabled);

                    break;
                }
                case GLFW_KEY_F8:
                    Quit();
                    break;
            }
        }
    });

    initResources();

    auto& settings = m_renderer.GetSettings();
    settings.Exposure = 1.0f;
    settings.Tonemapper = ToneMapping::ACES;

    m_scene.GetEnvironment()->Intensity = 0.2f;

    m_camera.SetPosition({0.0f, 1.0f, 3.0f});

    for (auto& probe : m_scene.GetReflectionProbes()) {
        m_renderer.BakeReflectionProbe(m_scene, probe);
    }

    Input::SetCursorMode(CursorMode::Disabled);
}

void Game::Update(float deltatime) {
    updateCamera(deltatime);
}

void Game::Render() {
    m_renderer.Render(m_scene, m_camera, m_clearColor);
}

void Game::Destroy() {
    m_scene.Clear();
    m_renderer.Destroy();
}

void Game::showInfo() {
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);

    LogInfo("GPU: {} - {}", vendor, renderer);
    LogInfo("OpenGL: {}", version);
}

void Game::initResources() {
    auto environment = ResourceManager::LoadEnvironment("assets/hdr/sky.hdr");
    m_scene.SetEnvironment(environment);

    auto sponzaModel = ResourceManager::LoadModel("assets/models/sponza.glb");
    if (!sponzaModel) {
        LogError("Failed to load sponza model");
        return;
    }

    auto helmetModel = ResourceManager::LoadModel("assets/models/damaged_helmet.glb");
    if (!helmetModel) {
        LogError("Failed to load helmet model");
        return;
    }

    GameObject& sponza = m_scene.CreateGameObject("Sponza");
    sponza.SetModel(sponzaModel);

    sponza.GetTransform().SetEulerRotation({0.0f, 90.0f, 0.0f});

    GameObject& helmet = m_scene.CreateGameObject("Helmet");
    helmet.SetModel(helmetModel);

    auto& helmetTransform = helmet.GetTransform();
    helmetTransform.Position.y += 1.5f;
    helmetTransform.Scale *= 0.8f;

    m_scene.GetSun().Intensity = 1.0f;

    m_scene.AddPointLight({
        .Intensity = 10.0f,
        .Tint = {1.0f, 0.0f, 0.0f},
        .Position = {-2.0f, 2.0f, 0.0f},
        .Radius = 5.0f
    });

    m_scene.AddPointLight({
        .Intensity = 10.0f,
        .Tint = {0.0f, 0.0f, 1.0f},
        .Position = {2.0f, 2.0f, 0.0f},
        .Radius = 5.0f
    });

    m_scene.AddSpotLight({
        .Intensity = 10.0f,
        .Tint = {0.0f, 1.0f, 0.0f},
        .Position = {0.0f, 3.0f, 0.0f},
        .Direction = {0.0f, -1.0f, 0.0f},
        .Radius = 10.0f,
        .InnerCone = 20.0f,
        .OuterCone = 30.0f
    });

    auto& probeA = m_scene.AddReflectionProbe({
        .Position = {-2.0f, 2.0f, 0.0f},
        .BoxMin = {-4.0f, 0.0f, -3.0f},
        .BoxMax = { 0.5f, 4.0f,  3.0f},
        .Intensity = 1.0f,
        .BlendDistance = 1.0f
    });

    auto& probeB = m_scene.AddReflectionProbe({
        .Position = {2.0f, 2.0f, 0.0f},
        .BoxMin = {-0.5f, 0.0f, -3.0f},
        .BoxMax = { 4.0f, 4.0f,  3.0f},
        .Intensity = 1.0f,
        .BlendDistance = 1.0f
    });
}

void Game::updateCamera(float deltatime) {
    if (!Input::IsCursorCaptured() || !Input::IsCursorInside()) return;

    const glm::vec2 mouseDelta = Input::GetMouseDelta();

    constexpr float sensitivity = 0.12f;
    constexpr float moveSpeed = 4.0f;
    constexpr float sprintSpeed = 7.0f;

    constexpr float acceleration = 14.0f;
    constexpr float deceleration = 18.0f;

    // MOUSE LOOK
    m_cameraYaw += mouseDelta.x * sensitivity;
    m_cameraPitch -= mouseDelta.y * sensitivity;
    m_cameraPitch = glm::clamp(m_cameraPitch, -89.0f, 89.0f);

    const float yaw = glm::radians(m_cameraYaw);
    const float pitch = glm::radians(m_cameraPitch);

    glm::vec3 front {
        glm::cos(yaw) * glm::cos(pitch),
        glm::sin(pitch),
        glm::sin(yaw) * glm::cos(pitch)
    };

    front = glm::normalize(front);

    constexpr glm::vec3 worldUp{0.0f, 1.0f, 0.0f};

    const glm::vec3 right = glm::normalize(glm::cross(front, worldUp));

    // MOVEMENT
    glm::vec3 forward{front.x, 0.0f, front.z};
    if (glm::length(forward) > 0.0f) forward = glm::normalize(forward);

    glm::vec3 movement{0.0f};
    if (Input::IsKeyDown(GLFW_KEY_W)) movement += forward;
    if (Input::IsKeyDown(GLFW_KEY_S)) movement -= forward;
    if (Input::IsKeyDown(GLFW_KEY_D)) movement += right;
    if (Input::IsKeyDown(GLFW_KEY_A)) movement -= right;
    if (Input::IsKeyDown(GLFW_KEY_SPACE)) movement += worldUp;
    if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) movement -= worldUp;

    if (glm::length(movement) > 0.0f) movement = glm::normalize(movement);

    const float speed = Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT)
                        ? sprintSpeed
                        : moveSpeed;

    const glm::vec3 targetVelocity = movement * speed;

    // SMOOTH
    const float smoothness = glm::length(movement) > 0.0f
                             ? acceleration
                             : deceleration;

    const float t = 1.0f - std::exp(-smoothness * deltatime);

    m_cameraVelocity = glm::mix(m_cameraVelocity, targetVelocity, t);

    // UPDATE
    const glm::vec3 position = m_camera.GetPosition() + m_cameraVelocity * deltatime;
    m_camera.SetPosition(position);
    m_camera.LookAt(position + front);
}
