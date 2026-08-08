#include <platform/window.hpp>

#include <core/error_handling.hpp>
#include <core/logging.hpp>

#include <array>

void APIENTRY Window::openglDebugOutput(
    GLenum src, GLenum type,
    uint id,
    GLenum severity,
    GLsizei length,
    const char* msg, const void* userParam
) {
    std::array<uint, 4> ignoreIds {
        131169,
        131185,
        131218,
        131204
    };

    if (std::ranges::find(ignoreIds, id) != ignoreIds.end()) return;

    std::string srcStr = "";
    switch (src) {
        case GL_DEBUG_SOURCE_API: srcStr.append("Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: srcStr.append("Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: srcStr.append("Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: srcStr.append("Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION: srcStr.append("Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER: srcStr.append("Source: Other"); break;
    }

    std::string typeStr = "";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: typeStr.append("Type: Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr.append("Type: Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr.append("Type: Undefined Behaviour"); break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr.append("Type: Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr.append("Type: Performance"); break;
        case GL_DEBUG_TYPE_MARKER: typeStr.append("Type: Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP: typeStr.append("Type: Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP: typeStr.append("Type: Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER: typeStr.append("Type: Other"); break;
    }

    std::string severityStr = "";
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: severityStr.append("Severity: high"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr.append("Severity: medium"); break;
        case GL_DEBUG_SEVERITY_LOW: severityStr.append("Severity: low"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr.append("Severity: notification"); break;
    }

    LogError("Source: {}\nType: {}\nSeverity: {}", srcStr, typeStr, severityStr);
}

void Window::glfwFramebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

    window->m_properties.Width = static_cast<uint32_t>(width);
    window->m_properties.Height = static_cast<uint32_t>(height);

    glViewport(0, 0, width, height);
}

Window::Window(const WindowProperties& properties)
    : m_properties(properties) {
    CheckError(glfwInit(), "GLFW", "Couldn't initialize GLFW!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    m_window = glfwCreateWindow(
        m_properties.Width, m_properties.Height,
        m_properties.Title.c_str(),
        nullptr, nullptr
    );

    CheckError(m_window != nullptr, "GLFW", "Couldn't create window!");

    glfwMakeContextCurrent(m_window);

    CheckError(gladLoadGL((GLADloadfunc)glfwGetProcAddress), "OpenGL", "Couldn't initialize OpenGL!");

    setupDebugMessenger();

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, glfwFramebufferSizeCallback);

    glViewport(0, 0, static_cast<int>(m_properties.Width), static_cast<int>(m_properties.Height));

    LogInfo("Created Window ({}x{})", m_properties.Width, m_properties.Height);
}

Window::~Window() {
    if (m_window) glfwDestroyWindow(m_window);
    glfwTerminate();

    LogInfo("Destroyed window");
}

void Window::setupDebugMessenger() {
    int flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglDebugOutput, nullptr);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    }
}
