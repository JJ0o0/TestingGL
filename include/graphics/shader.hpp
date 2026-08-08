#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <graphics/color.hpp>

#include <filesystem>
#include <cstdint>

class Shader {
    public:
        Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
        ~Shader();

        void Bind() const;

        void SetInt(const std::string& name, int value);
        void SetMat4(const std::string& name, const glm::mat4& value);
        void SetVec4(const std::string& name, const glm::vec4& value);
        void SetColor(const std::string& name, const Color& value);

        bool UniformExists(const std::string& name) { return getUniformLocationSilent(name) != 1; }
    private:
        uint32_t m_id = 0;

        uint32_t compileShader(const char* code, GLenum type);
        uint32_t compileProgram(uint32_t vertexShader, uint32_t fragmentShader);

        bool isShaderCompilationSuccessful(uint32_t shader, GLenum type);
        bool isProgramLinkingSuccessful(uint32_t program);

        int getUniformLocation(const std::string& name);
        int getUniformLocationSilent(const std::string& name);

        std::string readShaderFile(const std::filesystem::path& path);
};
