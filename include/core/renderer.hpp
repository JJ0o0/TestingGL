#pragma once

#include <platform/window.hpp>

#include <graphics/premade_meshes/cube.hpp>
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
        Renderer(Window& window) : m_window(window) {
            m_skyboxMesh = CreateCubemapCube();

            m_pbrShader = std::make_shared<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");

            m_skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
            m_skyboxShader->SetInt("uEnvironmentMap", 0);

            m_brdfLUT = CreateBRDFLUT(512);
        }

        void Render(
            const Scene& scene,
            const Camera& camera,
            const Color& clearColor
        );

        void Destroy();
    private:
        Window& m_window;

        std::shared_ptr<Mesh> m_skyboxMesh;

        std::shared_ptr<Shader> m_pbrShader;
        std::shared_ptr<Shader> m_skyboxShader;

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
