#include "Camera.h"

Camera::Camera()
    : Position(glm::vec3(0.0f, 0.0f, 0.0f)),
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    Up(glm::vec3(0.0f, 1.0f, 0.0f)),
    WorldUp(Up),
    Yaw(-90.0f),
    Pitch(0.0f),
    MovementSpeed(2.5f),
    MouseSensitivity(0.1f),
    Zoom(45.0f) {
    updateCameraVectors();
}

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Position(position),
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    Up(up),
    WorldUp(up),
    Yaw(yaw),
    Pitch(pitch),
    MovementSpeed(2.5f),
    MouseSensitivity(0.1f),
    Zoom(45.0f) {
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

float Camera::GetZoom() const { // Implementação do método GetZoom
    return Zoom;
}

glm::vec3 Camera::GetPosition() const { // Implementação do método GetPosition
    return Position;
}

void Camera::SetPosition(glm::vec3 position) { // Implementação do método SetPosition
    Position = position;
    updateCameraVectors();
}

void Camera::ProcessKeyboard(const std::string& direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == "FORWARD")
        Position += Front * velocity;
    if (direction == "BACKWARD")
        Position -= Front * velocity;
    if (direction == "LEFT")
        Position -= Right * velocity;
    if (direction == "RIGHT")
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    Yaw += xOffset;
    Pitch += yOffset;

    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yOffset) {
    Zoom -= yOffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

