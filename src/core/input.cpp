#include <core/input.hpp>

GLFWwindow* Input::s_window = nullptr;

std::array<Input::InputState, GLFW_KEY_LAST + 1> Input::s_keys{};
std::array<Input::InputState, GLFW_MOUSE_BUTTON_LAST + 1> Input::s_mouseButtons{};

Input::KeyCallback Input::s_keyCallback{};
Input::KeyCallback Input::s_mouseButtonCallback{};

CursorMode Input::s_cursorMode{};
glm::vec2 Input::s_mousePosition{};
glm::vec2 Input::s_mouseDelta{};
float Input::s_scrollDelta = 0.0f;
bool Input::s_firstMouse = true;
bool Input::s_cursorInside = false;

void Input::Initialize(GLFWwindow* window) {
    s_window = window;

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseCursorPosCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetCursorEnterCallback(window, mouseCursorEnterCallback);
}

void Input::BeginFrame() {
    s_mouseDelta = {};
    s_scrollDelta = 0.0f;

    for (auto& state : s_keys) {
        state.Pressed = false;
        state.Released = false;
    }

    for (auto& state : s_mouseButtons) {
        state.Pressed = false;
        state.Released = false;
    }
}

void Input::Shutdown() {
    s_window = nullptr;
}

bool Input::IsKeyDown(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return s_keys[key].Down;
}

bool Input::IsKeyPressed(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return s_keys[key].Pressed;
}

bool Input::IsKeyReleased(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return s_keys[key].Released;
}

bool Input::IsMouseButtonDown(int btn) {
    if (btn < 0 || btn > GLFW_MOUSE_BUTTON_LAST) return false;
    return s_mouseButtons[btn].Down;
}

bool Input::IsMouseButtonPressed(int btn) {
    if (btn < 0 || btn > GLFW_MOUSE_BUTTON_LAST) return false;
    return s_mouseButtons[btn].Pressed;
}

bool Input::IsMouseButtonReleased(int btn) {
    if (btn < 0 || btn > GLFW_MOUSE_BUTTON_LAST) return false;
    return s_mouseButtons[btn].Released;
}

void Input::SetKeyCallback(KeyCallback callback) {
    s_keyCallback = std::move(callback);
}

void Input::SetMouseButtonCallback(MouseButtonCallback callback) {
    s_mouseButtonCallback = std::move(callback);
}

void Input::SetCursorMode(CursorMode mode) {
    if (!s_window) return;

    switch (mode) {
        case CursorMode::Normal:
            glfwSetInputMode(s_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case CursorMode::Hidden:
            glfwSetInputMode(s_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            break;
        case CursorMode::Disabled:
            glfwSetInputMode(s_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
    }

    s_cursorMode = mode;
    s_mouseDelta = {};
    s_firstMouse = true;
}

void Input::keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods) {
    if (key < 0 || key > GLFW_KEY_LAST) return;

    auto& state = s_keys[key];
    switch (action) {
        case GLFW_PRESS:
            state.Down = true;
            state.Pressed = true;

            if (s_keyCallback) s_keyCallback(key, InputAction::Pressed);
            break;
        case GLFW_RELEASE:
            state.Down = false;
            state.Released = true;

            if (s_keyCallback) s_keyCallback(key, InputAction::Released);
            break;
        case GLFW_REPEAT:
            state.Down = true;
            break;
    }
}

void Input::mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;

    auto& state = s_mouseButtons[button];
    switch (action) {
        case GLFW_PRESS:
            state.Down = true;
            state.Pressed = true;

            if (s_mouseButtonCallback) s_mouseButtonCallback(button, InputAction::Pressed);
            break;
        case GLFW_RELEASE:
            state.Down = false;
            state.Released = true;

            if (s_mouseButtonCallback) s_mouseButtonCallback(button, InputAction::Released);
            break;
        case GLFW_REPEAT:
            state.Down = true;
            break;
    }
}

void Input::mouseCursorPosCallback(GLFWwindow* glfwWindow, double x, double y) {
    glm::vec2 newPos {
        static_cast<float>(x),
        static_cast<float>(y)
    };

    if (s_firstMouse) {
        s_mousePosition = newPos;
        s_firstMouse = false;
        return;
    }

    s_mouseDelta += newPos - s_mousePosition;
    s_mousePosition = newPos;
}

void Input::mouseScrollCallback(GLFWwindow* glfwWindow, double xOffset, double yOffset) {
    s_scrollDelta += static_cast<float>(yOffset);
}

void Input::mouseCursorEnterCallback(GLFWwindow* glfwWindow, int entered) {
    s_cursorInside = entered == GLFW_TRUE;
    if (!s_cursorInside) {
        s_mouseDelta = {};
        s_firstMouse = true;
    }
}
