#pragma once

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include <functional>
#include <array>

enum class InputAction {
    Pressed,
    Released
};

enum class CursorMode {
    Normal,
    Hidden,
    Disabled
};

class Input {
    public:
        using KeyCallback = std::function<void(int key, InputAction action)>;
        using MouseButtonCallback = std::function<void(int btn, InputAction action)>;

        static void Initialize(GLFWwindow* window);
        static void BeginFrame();
        static void Shutdown();

        static bool IsKeyDown(int key);
        static bool IsKeyPressed(int key);
        static bool IsKeyReleased(int key);

        static bool IsMouseButtonDown(int btn);
        static bool IsMouseButtonPressed(int btn);
        static bool IsMouseButtonReleased(int btn);

        static inline CursorMode GetCursorMode() { return s_cursorMode; }
        static inline glm::vec2 GetMousePosition() { return s_mousePosition; }
        static inline glm::vec2 GetMouseDelta() { return s_mouseDelta; }
        static inline float GetScrollDelta() { return s_scrollDelta; }

        static inline bool IsCursorCaptured() { return s_cursorMode == CursorMode::Disabled; }
        static inline bool IsCursorInside() { return s_cursorInside; }

        static void SetKeyCallback(KeyCallback callback);
        static void SetMouseButtonCallback(MouseButtonCallback callback);
        static void SetCursorMode(CursorMode mode);
    private:
        struct InputState {
            bool Down = false;
            bool Pressed = false;
            bool Released = false;
        };

        static GLFWwindow* s_window;

        static std::array<InputState, GLFW_KEY_LAST + 1> s_keys;
        static std::array<InputState, GLFW_MOUSE_BUTTON_LAST + 1> s_mouseButtons;

        static CursorMode s_cursorMode;
        static glm::vec2 s_mousePosition;
        static glm::vec2 s_mouseDelta;
        static float s_scrollDelta;
        static bool s_firstMouse;
        static bool s_cursorInside;

        static KeyCallback s_keyCallback;
        static MouseButtonCallback s_mouseButtonCallback;

        static void keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
        static void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
        static void mouseCursorPosCallback(GLFWwindow* glfwWindow, double x, double y);
        static void mouseScrollCallback(GLFWwindow* glfwWindow, double xOffset, double yOffset);
        static void mouseCursorEnterCallback(GLFWwindow* glfwWindow, int entered);
};
