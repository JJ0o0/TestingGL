#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct Transform {
    glm::vec3 Position{0.0f};
    glm::quat Rotation = glm::quat_identity<float, glm::highp>();
    glm::vec3 Scale{1.0f};

    void Rotate(const glm::vec3& degrees) {
        const glm::quat delta = glm::quat(glm::radians(degrees));
        Rotation = glm::normalize(Rotation * delta);
    }

    void SetEulerRotation(const glm::vec3& degrees) {
        Rotation = glm::quat(glm::radians(degrees));
    }

    glm::vec3 GetEulerRotation() const {
        return glm::degrees(glm::eulerAngles(Rotation));
    }

    glm::mat4 GetModelMatrix() const {
        glm::mat4 model{1.0f};
        model = glm::translate(model, Position);
        model *= glm::mat4_cast(Rotation);
        model = glm::scale(model, Scale);

        return model;
    }
};
