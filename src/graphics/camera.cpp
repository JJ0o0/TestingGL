#include <graphics/camera.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

glm::mat4 Camera::GetView() const {
    return glm::lookAt(
        m_position,
        m_position + m_front,
        m_worldUp
    );
}

glm::mat4 Camera::GetProjection(float aspectRatio) const {
    return glm::perspective(
        m_properties.FieldOfView,
        aspectRatio,
        m_properties.Near, m_properties.Far
    );
}
