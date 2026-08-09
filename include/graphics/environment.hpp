#pragma once

#include <core/logging.hpp>

#include <graphics/premade_meshes/cube.hpp>
#include <graphics/texture.hpp>
#include <graphics/cubemap.hpp>
#include <graphics/shader.hpp>

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <memory>
#include <array>

inline std::shared_ptr<Cubemap> CreateEnvironmentCubemap(const Texture& equirectangularHDR, uint32_t size = 512) {
    auto cubemap = std::make_shared<Cubemap>(size, GL_RGB16F, GL_RGB, GL_FLOAT, false);

    uint32_t captureFBO = 0, captureRBO = 0;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    const glm::mat4 captureProjection = glm::perspective(
        glm::radians(90.0f),
        1.0f,
        0.1f, 10.0f
    );

    const std::array<glm::mat4, 6> captureViews = {
        glm::lookAt(
            glm::vec3(0),
            glm::vec3( 1, 0, 0),
            glm::vec3( 0,-1, 0)
        ),
        glm::lookAt(
            glm::vec3(0),
            glm::vec3(-1, 0, 0),
            glm::vec3( 0,-1, 0)
        ),
        glm::lookAt(
            glm::vec3(0),
            glm::vec3(0, 1, 0),
            glm::vec3(0, 0, 1)
        ),
        glm::lookAt(
            glm::vec3(0),
            glm::vec3(0,-1, 0),
            glm::vec3(0, 0,-1)
        ),
        glm::lookAt(
            glm::vec3(0),
            glm::vec3(0, 0, 1),
            glm::vec3(0,-1, 0)
        ),
        glm::lookAt(
            glm::vec3(0),
            glm::vec3(0, 0,-1),
            glm::vec3(0,-1, 0)
        )
    };

    auto cube = CreateCubemapCube();

    Shader conversionShader("assets/shaders/ibl/equirectangular_to_cubemap.vert", "assets/shaders/ibl/equirectangular_to_cubemap.frag");
    conversionShader.Bind();

    conversionShader.SetInt("uEquirectangularMap", 0);
    conversionShader.SetMat4("uProjection", captureProjection);
    equirectangularHDR.Bind(0);

    int previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);
    glViewport(0, 0, size, size);

    const GLboolean cullingEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    for (uint32_t face = 0; face < 6; ++face) {
        conversionShader.SetMat4("uView", captureViews[face]);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            cubemap->GetID(),
            0
        );

        if (face == 0 && glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LogError("Environment capture framebuffer is incomplete");
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube->Draw();
    }

    if (cullingEnabled) glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(
        previousViewport[0],
        previousViewport[1],
        previousViewport[2],
        previousViewport[3]
    );

    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteFramebuffers(1, &captureFBO);

    return cubemap;
}
