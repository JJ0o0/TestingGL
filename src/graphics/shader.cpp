#include <graphics/shader.hpp>

#include <core/logging.hpp>

#include <fstream>
#include <sstream>
#include <string>

Shader::Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath) {
    const std::string vertexCodeStr = readShaderFile(vertexPath);
    const std::string fragmentCodeStr = readShaderFile(fragmentPath);

    const char* vertexCode = vertexCodeStr.c_str();
    const char* fragmentCode = fragmentCodeStr.c_str();

    uint32_t vertexShader = compileShader(vertexCode, GL_VERTEX_SHADER);
    uint32_t fragmentShader = compileShader(fragmentCode, GL_FRAGMENT_SHADER);

    if (!vertexShader || !fragmentShader) {
        if (vertexShader) glDeleteShader(vertexShader);
        if (fragmentShader) glDeleteShader(fragmentShader);

        return;
    }

    m_id = compileProgram(vertexShader, fragmentShader);
}

Shader::~Shader() { if (m_id) glDeleteProgram(m_id); }

void Shader::Bind() const { glUseProgram(m_id); }

uint32_t Shader::compileShader(const char* code, GLenum type) {
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);

    if (!isShaderCompilationSuccessful(shader, type)) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

uint32_t Shader::compileProgram(uint32_t vertexShader, uint32_t fragmentShader) {
    uint32_t program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    if (!isProgramLinkingSuccessful(program)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

bool Shader::isShaderCompilationSuccessful(uint32_t shader, GLenum type) {
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success) return true;

    int logLength = 512;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    std::string log(logLength, '\0');
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());

    std::string typeStr = "Unknown Shader Type";
    switch (type) {
        case GL_VERTEX_SHADER: typeStr = "Vertex Shader"; break;
        case GL_FRAGMENT_SHADER: typeStr = "Fragment Shader"; break;
        case GL_GEOMETRY_SHADER: typeStr = "Geometry Shader"; break;
    }

    LogWarning("{} Compilation failed: {}", typeStr, log);
    return false;
}

bool Shader::isProgramLinkingSuccessful(uint32_t program) {
    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success) return true;

    int logLength = 512;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

    std::string log(logLength, '\0');
    glGetProgramInfoLog(program, logLength, nullptr, log.data());

    LogWarning("Shader Program Linking failed: {}", log);
    return false;
}

std::string Shader::readShaderFile(const std::filesystem::path& path) {
    std::ifstream file(path);

    if (!file) {
        LogWarning("Couldn't open shader file at {}!", path.string());
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    if (file.bad()) {
        LogWarning("Couldn't read shader file at {}!", path.string());
        return {};
    }

    return buffer.str();
}
