#pragma once

#include <core/render_settings.hpp>
#include <core/logging.hpp>

#include <platform/window.hpp>

#include <graphics/premade_meshes/screen_quad.hpp>
#include <graphics/premade_meshes/cube.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics/environment.hpp>
#include <graphics/cubemap.hpp>
#include <graphics/texture.hpp>
#include <graphics/camera.hpp>
#include <graphics/light.hpp>
#include <graphics/color.hpp>
#include <graphics/model.hpp>
#include <graphics/shader.hpp>

#include <world/gameobject.hpp>
#include <world/scene.hpp>

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
        Window& m_window;
        RenderSettings m_settings{};

        std::unique_ptr<Framebuffer> m_hdrFramebuffer;

        std::shared_ptr<Mesh> m_skyboxMesh;
        std::shared_ptr<Mesh> m_screenQuad;

        std::shared_ptr<Shader> m_pbrShader;
        std::shared_ptr<Shader> m_skyboxShader;
        std::shared_ptr<Shader> m_postProcessShader;

        std::shared_ptr<Texture> m_brdfLUT;

        void drawObject(
            const GameObject& object,
            const Camera& camera,
            const DirectionalLight& sun
        );

        void drawModelNode(
            const Model& model,
            uint32_t nodeIndex,
            const glm::mat4& parentTransform,
            const Camera& camera,
            const DirectionalLight& sun
        );

        void drawSkybox(
            const Cubemap& cubemap,
            const Camera& camera
        );
};
