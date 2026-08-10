#pragma once

#include <core/logging.hpp>

#include <graphics/premade_meshes/screen_quad.hpp>
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
    auto cubemap = std::make_shared<Cubemap>(size, GL_RGB16F, GL_RGB, GL_FLOAT, true);

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
        glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3( 0, 0, 1)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3( 0, 0,-1)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3( 0,-1, 0))
    };

    auto cube = CreateCubemapCube();

    Shader conversionShader("assets/shaders/ibl/cubemap_capture.vert", "assets/shaders/ibl/equirectangular_to_cubemap.frag");
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
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    for (uint32_t face = 0; face < 6; ++face) {
        conversionShader.SetMat4("uView", captureViews[face]);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            cubemap->GetID(),
            0
        );

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
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

    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->GetID());
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return cubemap;
}

inline std::shared_ptr<Cubemap> CreateIrradianceCubemap(const Cubemap& environmentMap, uint32_t size = 512) {
    auto irradianceMap = std::make_shared<Cubemap>(size, GL_RGB16F, GL_RGB, GL_FLOAT, false);

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
        glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3( 0, 0, 1)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3( 0, 0,-1)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3( 0,-1, 0))
    };

    auto cube = CreateCubemapCube();

    Shader irradianceShader("assets/shaders/ibl/cubemap_capture.vert", "assets/shaders/ibl/irradiance_convolution.frag");
    irradianceShader.Bind();

    irradianceShader.SetInt("uEnvironmentMap", 0);
    irradianceShader.SetMat4("uProjection", captureProjection);
    environmentMap.Bind(0);

    int previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);
    glViewport(0, 0, size, size);

    const GLboolean cullingEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    for (uint32_t face = 0; face < 6; ++face) {
        irradianceShader.SetMat4("uView", captureViews[face]);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            irradianceMap->GetID(),
            0
        );

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
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

    return irradianceMap;
}

inline std::shared_ptr<Cubemap> CreatePrefilteredEnvironmentCubemap(const Cubemap& environmentMap, uint32_t size = 128) {
    auto prefilterMap = std::make_shared<Cubemap>(size, GL_RGB16F, GL_RGB, GL_FLOAT, true);

    uint32_t captureFBO = 0, captureRBO = 0;

    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    const glm::mat4 captureProjection = glm::perspective(
        glm::radians(90.0f),
        1.0f,
        0.1f, 10.0f
    );

    const std::array<glm::mat4, 6> captureViews = {
        glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3( 0, 0, 1)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3( 0, 0,-1)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3( 0,-1, 0))
    };

    auto cube = CreateCubemapCube();

    Shader prefilterShader("assets/shaders/ibl/cubemap_capture.vert", "assets/shaders/ibl/prefilter.frag");

    prefilterShader.Bind();
    prefilterShader.SetInt("uEnvironmentMap", 0);
    prefilterShader.SetMat4("uProjection", captureProjection);

    environmentMap.Bind(0);

    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);

    const GLboolean cullingEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    constexpr uint32_t maxMipLevels = 5;
    for (uint32_t mip = 0; mip < maxMipLevels; ++mip) {
        const uint32_t mipWidth = static_cast<uint32_t>(size * std::pow(0.5f, static_cast<float>(mip)));
        const uint32_t mipHeight = static_cast<uint32_t>(size * std::pow(0.5f, static_cast<float>(mip)));

        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

        glViewport(0, 0, mipWidth, mipHeight);

        const float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        prefilterShader.SetFloat("uRoughness", roughness);

        for (uint32_t face = 0; face < 6; ++face) {
            prefilterShader.SetMat4("uView", captureViews[face]);

            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                prefilterMap->GetID(),
                mip
            );

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                LogError("Prefilter framebuffer incomplete on mip {} face {}", mip, face);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube->Draw();
        }
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

    return prefilterMap;
}

inline std::shared_ptr<Texture> CreateBRDFLUT(uint32_t size = 512) {
    auto brdfLUT = std::make_shared<Texture>(size, size, GL_RG16F, GL_RG, GL_FLOAT);

    uint32_t captureFBO = 0, captureRBO = 0;

    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT->GetID(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) LogError("BRDF LUT framebuffer is incomplete");

    auto quad = CreateScreenQuad();

    Shader brdfShader("assets/shaders/ibl/brdf.vert", "assets/shaders/ibl/brdf.frag");

    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);
    glViewport(0, 0, size, size);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    brdfShader.Bind();
    quad->Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(
        previousViewport[0],
        previousViewport[1],
        previousViewport[2],
        previousViewport[3]
    );

    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteFramebuffers(1, &captureFBO);

    return brdfLUT;
}
