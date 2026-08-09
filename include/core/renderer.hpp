#pragma once

#include <platform/window.hpp>

#include <graphics/premade_meshes/cube.hpp>
#include <graphics/cubemap.hpp>
#include <graphics/camera.hpp>
#include <graphics/light.hpp>
#include <graphics/color.hpp>
#include <graphics/model.hpp>

#include <world/gameobject.hpp>
#include <world/scene.hpp>

class Renderer {
    public:
        Renderer(Window& window) : m_window(window) {
            m_skyboxMesh = CreateCubemapCube();

            m_skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
            m_skyboxShader->SetInt("uEnvironmentMap", 0);
        }

        void Render(
            const Scene& scene,
            const Camera& camera,
            const Color& clearColor,
            const Cubemap& environmentMap
        );

        void Destroy();
    private:
        Window& m_window;

        std::shared_ptr<Mesh> m_skyboxMesh;
        std::shared_ptr<Shader> m_skyboxShader;

        void drawObject(
            const GameObject& object,
            const Camera& camera,
            const AmbientLight& ambient,
            const DirectionalLight& sun
        );

        void drawModelNode(
            const Model& model,
            uint32_t nodeIndex,
            const glm::mat4& parentTransform,
            const Camera& camera,
            const AmbientLight& ambient,
            const DirectionalLight& sun
        );

        void drawSkybox(
            const Cubemap& cubemap,
            const Camera& camera
        );
};
