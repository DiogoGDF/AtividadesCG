#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera();
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    glm::mat4 GetViewMatrix() const;
    float GetZoom() const; // Método para obter o Zoom
    glm::vec3 GetPosition() const; // Método para obter a posição
    glm::vec3 GetFront() const; // Método para obter a direção frontal
    void SetPosition(glm::vec3 position); // Método para definir a posição

    void ProcessKeyboard(const std::string& direction, float deltaTime);
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yOffset);

private:
    void updateCameraVectors();

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
};

#endif

