#include <graphics/environment.hpp>

#include <core/logging.hpp>

#include <graphics/premade_meshes/cube.hpp>
#include <graphics/shader.hpp>

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>

namespace {
    const glm::mat4 CaptureProjection = glm::perspective(
        glm::radians(90.0f),
        1.0f,
        0.1f,
        10.0f
    );

    const std::array<glm::mat4, 6> CaptureViews = {
        glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3( 0, 0, 1)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3( 0, 0,-1)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3( 0,-1, 0)),
        glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3( 0,-1, 0))
    };

    class CubemapCapture {
        public:
            CubemapCapture() {
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_previousFramebuffer);
                glGetIntegerv(GL_RENDERBUFFER_BINDING, &m_previousRenderbuffer);
                glGetIntegerv(GL_VIEWPORT, m_previousViewport.data());

                m_cullingEnabled = glIsEnabled(GL_CULL_FACE);

                glGenFramebuffers(1, &m_fbo);
                glGenRenderbuffers(1, &m_rbo);

                glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
                glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);

                glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_RENDERBUFFER,
                    m_rbo
                );

                glDrawBuffer(GL_COLOR_ATTACHMENT0);

                glDisable(GL_CULL_FACE);
            }

            ~CubemapCapture() {
                glBindFramebuffer(GL_FRAMEBUFFER, m_previousFramebuffer);
                glBindRenderbuffer(GL_RENDERBUFFER, m_previousRenderbuffer);

                glViewport(
                    m_previousViewport[0],
                    m_previousViewport[1],
                    m_previousViewport[2],
                    m_previousViewport[3]
                );

                if (m_cullingEnabled) glEnable(GL_CULL_FACE);
                else glDisable(GL_CULL_FACE);

                glDeleteRenderbuffers(1, &m_rbo);
                glDeleteFramebuffers(1, &m_fbo);
            }

            CubemapCapture(const CubemapCapture&) = delete;
            CubemapCapture& operator=(const CubemapCapture&) = delete;

            void Resize(uint32_t width, uint32_t height) {
                glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

                glViewport(0, 0, width, height);
            }

            void AttachFace(const Cubemap& cubemap, uint32_t face, uint32_t mip = 0) {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                    cubemap.GetID(),
                    mip
                );
            }

            bool IsComplete() const { return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE; }
        private:
            GLuint m_fbo = 0;
            GLuint m_rbo = 0;

            GLint m_previousFramebuffer = 0;
            GLint m_previousRenderbuffer = 0;

            std::array<GLint, 4> m_previousViewport{};

            GLboolean m_cullingEnabled = GL_FALSE;
    };

    std::shared_ptr<Cubemap> CreateEnvironmentCubemap(const Texture& equirectangularHDR, uint32_t size) {
        auto cubemap = std::make_shared<Cubemap>(size, GL_RGB16F, GL_RGB, GL_FLOAT, true);

        CubemapCapture capture;
        capture.Resize(size, size);

        auto cube = CreateCubemapCube();

        Shader conversionShader("assets/shaders/ibl/cubemap_capture.vert", "assets/shaders/ibl/equirectangular_to_cubemap.frag");
        conversionShader.Bind();

        conversionShader.SetInt("uEquirectangularMap", 0);
        conversionShader.SetMat4("uProjection", CaptureProjection);
        equirectangularHDR.Bind(0);

        for (uint32_t face = 0; face < 6; ++face) {
            conversionShader.SetMat4("uView", CaptureViews[face]);

            capture.AttachFace(*cubemap, face);
            if (!capture.IsComplete()) {
                LogError("Environment framebuffer incomplete on face {}", face);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube->Draw();
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->GetID());
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        return cubemap;
    }
}

std::shared_ptr<Cubemap> CreateIrradianceCubemap(const Cubemap& environmentMap, uint32_t size) {
    auto irradianceMap = std::make_shared<Cubemap>(size, GL_RGB16F, GL_RGB, GL_FLOAT, false);

    CubemapCapture capture;
    capture.Resize(size, size);

    auto cube = CreateCubemapCube();

    Shader irradianceShader("assets/shaders/ibl/cubemap_capture.vert", "assets/shaders/ibl/irradiance_convolution.frag");
    irradianceShader.Bind();

    irradianceShader.SetInt("uEnvironmentMap", 0);
    irradianceShader.SetMat4("uProjection", CaptureProjection);
    environmentMap.Bind(0);

    for (uint32_t face = 0; face < 6; ++face) {
        irradianceShader.SetMat4("uView", CaptureViews[face]);

        capture.AttachFace(*irradianceMap, face);
        if (!capture.IsComplete()) {
            LogError("Irradiance framebuffer is incomplete on face {}", face);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube->Draw();
    }

    return irradianceMap;
}

std::shared_ptr<Cubemap> CreatePrefilteredEnvironmentCubemap(const Cubemap& environmentMap, uint32_t size) {
    auto prefilterMap = std::make_shared<Cubemap>(size, GL_RGB16F, GL_RGB, GL_FLOAT, true);

    CubemapCapture capture;

    auto cube = CreateCubemapCube();

    Shader prefilterShader("assets/shaders/ibl/cubemap_capture.vert", "assets/shaders/ibl/prefilter.frag");

    prefilterShader.Bind();
    prefilterShader.SetInt("uEnvironmentMap", 0);
    prefilterShader.SetMat4("uProjection", CaptureProjection);

    environmentMap.Bind(0);

    constexpr uint32_t maxMipLevels = 5;
    for (uint32_t mip = 0; mip < maxMipLevels; ++mip) {
        const uint32_t mipWidth = std::max(1u, size >> mip);
        const uint32_t mipHeight = std::max(1u, size >> mip);
        capture.Resize(mipWidth, mipHeight);

        const float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        prefilterShader.SetFloat("uRoughness", roughness);

        for (uint32_t face = 0; face < 6; ++face) {
            prefilterShader.SetMat4("uView", CaptureViews[face]);

            capture.AttachFace(*prefilterMap, face, mip);

            if (!capture.IsComplete()) {
                LogError("Prefilter framebuffer incomplete on mip {} face {}", mip, face);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube->Draw();
        }
    }

    return prefilterMap;
}

std::shared_ptr<Environment> CreateEnvironment(
    const Texture& equirectangularHDR,
    uint32_t environmentSize,
    uint32_t irradianceSize,
    uint32_t prefilterSize
) {
    auto environment = std::make_shared<Environment>();
    environment->Skybox = CreateEnvironmentCubemap(equirectangularHDR, environmentSize);
    environment->Irradiance = CreateIrradianceCubemap(*environment->Skybox, irradianceSize);
    environment->Prefilter = CreatePrefilteredEnvironmentCubemap(*environment->Skybox, prefilterSize);

    return environment;
}
