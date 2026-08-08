#pragma once

#include <glad/gl.h>

#include <filesystem>
#include <cstdint>

class Shader {
    public:
        Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
        ~Shader();

        void Bind() const;
    private:
        uint32_t m_id = 0;

        uint32_t compileShader(const char* code, GLenum type);
        uint32_t compileProgram(uint32_t vertexShader, uint32_t fragmentShader);

        bool checkShaderCompilationError(uint32_t shader, GLenum type);
        bool checkProgramLinkingError(uint32_t program);

        std::string readShaderFile(const std::filesystem::path& path);
};
