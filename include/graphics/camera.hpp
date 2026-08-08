#pragma once

#include <glm/glm.hpp>

struct CameraProperties {
    float FieldOfView = 75.0f;
    float Near = 0.1f;
    float Far = 100.0f;
};

class Camera {
    public:
        Camera(const CameraProperties& properties = {}) : m_properties(properties) {}

        void LookAt(const glm::vec3& target) { m_front = glm::normalize(target - m_position); }

        glm::mat4 GetView() const;
        glm::mat4 GetProjection(float aspectRatio) const;

        void SetPosition(const glm::vec3& position) { m_position = position; }
        const glm::vec3& GetPosition() const { return m_position; }

        const CameraProperties& GetProperties() const { return m_properties; }
    private:
        CameraProperties m_properties{};

        glm::vec3 m_position{0.0f, 0.0f, 3.0f};
        glm::vec3 m_front{0.0f, 0.0f, -1.0f};
        glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};
};
