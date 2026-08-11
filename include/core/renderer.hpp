#pragma once

#include <core/render_settings.hpp>
#include <core/logging.hpp>

#include <platform/window.hpp>

#include <graphics/premade_meshes/screen_quad.hpp>
#include <graphics/premade_meshes/cube.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics/environment.hpp>
#include <graphics/shadow_map.hpp>
#include <graphics/cubemap.hpp>
#include <graphics/texture.hpp>
#include <graphics/gbuffer.hpp>
#include <graphics/camera.hpp>
#include <graphics/light.hpp>
#include <graphics/color.hpp>
#include <graphics/model.hpp>
#include <graphics/shader.hpp>

#include <world/gameobject.hpp>
#include <world/scene.hpp>

#include <vector>

class Renderer {
    public:
        Renderer(Window& window);

        void Render(
            const Scene& scene,
            const Camera& camera,
            const Color& clearColor
        );

        void Destroy();

        RenderSettings& GetSettings() { return m_settings; }
        const RenderSettings& GetSettings() const { return m_settings; }
    private:
        struct RenderItem {
            const Mesh* Geometry = nullptr;
            const Material* MaterialData = nullptr;
            glm::mat4 ModelMatrix{1.0f};
        };

        Window& m_window;
        RenderSettings m_settings{};

        std::unique_ptr<GBuffer> m_gBuffer;
        std::unique_ptr<Framebuffer> m_hdrFramebuffer;

        std::shared_ptr<Mesh> m_skyboxMesh;
        std::shared_ptr<Mesh> m_screenQuad;

        std::shared_ptr<Shader> m_pbrShader;
        std::shared_ptr<Shader> m_unlitShader;
        std::shared_ptr<Shader> m_skyboxShader;
        std::shared_ptr<Shader> m_shadowShader;
        std::shared_ptr<Shader> m_geometryShader;
        std::shared_ptr<Shader> m_postProcessShader;
        std::shared_ptr<Shader> m_deferredLightingShader;

        std::unique_ptr<ShadowMap> m_shadowMap;
        std::shared_ptr<Texture> m_brdfLUT;

        void resizeRenderTargets(uint32_t width, uint32_t height);
        glm::mat4 calculateLightSpaceMatrix(const DirectionalLight& sun);

        void renderShadowPass(const std::vector<RenderItem>& items, const glm::mat4& lightSpaceMatrix);
        void renderGeometryPass(const std::vector<RenderItem>& items, const Camera& camera);
        void renderDeferredLightingPass(
            const Scene& scene,
            const Camera& camera,
            const glm::mat4& lightSpaceMatrix,
            const Color& clearColor
        );
        void renderForwardPass(const std::vector<RenderItem>& items, const Camera& camera);
        void renderPostProcessPass();

        std::vector<RenderItem> buildRenderItems(const Scene& scene) const;

        void collectModelNode(
            const Model& model,
            uint32_t nodeIndex,
            const glm::mat4& parentTransform,
            std::vector<RenderItem>& items
        ) const;

        void drawSkybox(const Cubemap& cubemap, const Camera& camera);
};
