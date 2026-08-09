#pragma once

#include <platform/window.hpp>

#include <graphics/camera.hpp>
#include <graphics/light.hpp>
#include <graphics/color.hpp>
#include <graphics/model.hpp>

#include <world/gameobject.hpp>
#include <world/scene.hpp>

class Renderer {
    public:
        Renderer(Window& window) : m_window(window) {}

        void Render(
            const Scene& scene,
            const Camera& camera,
            const Color& clearColor
        );
    private:
        Window& m_window;

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
};
