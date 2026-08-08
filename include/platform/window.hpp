#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

struct WindowProperties {
    std::string Title = "Testing Zone";
    uint32_t Width = 800;
    uint32_t Height = 600;
};

class Window {
    public:
        Window(const WindowProperties& properties = {});
        ~Window();

        bool ShouldClose() const { return glfwWindowShouldClose(m_window); }

        void PollEvents() const { glfwPollEvents(); }
        void SwapBuffers() const { glfwSwapBuffers(m_window); }

        const WindowProperties& GetProperties() const { return m_properties; }
    private:
        WindowProperties m_properties;
        GLFWwindow* m_window = nullptr;

        void setupDebugMessenger();

        static void APIENTRY openglDebugOutput(
            GLenum src, GLenum type,
            uint id,
            GLenum severity,
            GLsizei length,
            const char* msg, const void* userParam
        );

        static void glfwFramebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height);
};
