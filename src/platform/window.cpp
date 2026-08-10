#include <platform/window.hpp>

#include <core/error_handling.hpp>
#include <core/logging.hpp>

#include <array>

static std::string_view DebugSourceToString(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window System";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION: return "Application";
        case GL_DEBUG_SOURCE_OTHER: return "Other";
        default: return "Unknown";
    }
}

static std::string_view DebugTypeToString(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behaviour";
        case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
        case GL_DEBUG_TYPE_MARKER: return "Marker";
        case GL_DEBUG_TYPE_PUSH_GROUP: return "Push Group";
        case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
        case GL_DEBUG_TYPE_OTHER: return "Other";
        default: return "Unknown";
    }
}

static std::string_view DebugSeverityToString(GLenum severity) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: return "High";
        case GL_DEBUG_SEVERITY_MEDIUM: return "Medium";
        case GL_DEBUG_SEVERITY_LOW: return "Low";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
        default: return "Unknown";
    }
}

void APIENTRY Window::openglDebugOutput(
    GLenum src, GLenum type,
    uint id,
    GLenum severity,
    GLsizei length,
    const char* msg, const void* userParam
) {
    static constexpr std::array<uint, 4> ignoreIds {
        131169,
        131185,
        131218,
        131204
    };

    if (std::ranges::find(ignoreIds, id) != ignoreIds.end()) return;

    const auto srcStr = DebugSourceToString(src);
    const auto typeStr = DebugTypeToString(type);
    const auto severityStr = DebugSeverityToString(severity);

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            LogError(
                "OpenGL [{}] {} / {}: {}",
                id, srcStr, typeStr, msg
            );
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
        case GL_DEBUG_SEVERITY_LOW:
            LogWarning(
                "OpenGL [{}] {} / {}: {}",
                id, srcStr, typeStr, msg
            );
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            LogInfo(
                "OpenGL [{}] {} / {}: {}",
                id, srcStr, typeStr, msg
            );
            break;
    }
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
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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
