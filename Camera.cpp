#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {

        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {

        glm::mat4 viewMatrix = glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraFrontDirection, this->cameraUpDirection); //<-vectorul UP

        return viewMatrix;
    }
    void Camera::printCameraPosition() {
        printf("%f %f %f\n", this->cameraPosition.x, this->cameraPosition.y, this->cameraPosition.z);
    }
    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed,int ShiftOn) {
            this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));

            float shift = 1;
            if (ShiftOn) {
                shift = 5.0f;
            }
            switch (direction) {

            case MOVE_FORWARD:
                this->cameraPosition += shift * speed * this->cameraFrontDirection;
                break;
            case MOVE_BACKWARD:
                this->cameraPosition -= shift * speed * this->cameraFrontDirection;
                break;
            case MOVE_RIGHT:
                this->cameraPosition += shift * this->cameraRightDirection * speed;
                break;
            case MOVE_LEFT:
                this->cameraPosition -= shift * this->cameraRightDirection * speed;
                break;
            default:
                break;
            }
    }
    void Camera::setCameraPosition(glm::vec3 v) {
        this->cameraPosition = v;
    }
 
    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    float yawangle;
    float pitchangle;
    void Camera::rotate(float pitch, float yaw) {

        glm::vec3 front;

        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraFrontDirection = glm::normalize(front);
    }
}