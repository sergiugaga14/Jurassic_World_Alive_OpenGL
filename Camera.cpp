#include "Camera.hpp"

namespace gps {

    
    
    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp)
    {


        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPosition);

        this->cameraFrontDirection = glm::normalize( cameraTarget-cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
        this->cameraUpDirection = cameraUp;
       
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {

        return glm::lookAt(cameraPosition,cameraPosition + cameraFrontDirection, cameraUpDirection);
    }
    

   
    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {

        if (direction == MOVE_UP)
            cameraPosition += speed * cameraUpDirection;
        if (direction == MOVE_DOWN)
            cameraPosition -= speed * cameraUpDirection;
        if (direction == MOVE_FORWARD)
            cameraPosition += speed * cameraFrontDirection;
        if (direction == MOVE_BACKWARD)
            cameraPosition -= speed * cameraFrontDirection;
        if (direction == MOVE_LEFT)
            cameraPosition -= glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
        if (direction == MOVE_RIGHT)
            cameraPosition += glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;   
       
       
       

    }
    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::setCameraPosition(glm::vec3 pos)
    {
        this->cameraPosition = pos;
    }

    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 direction;

        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        this->cameraFrontDirection = glm::normalize(direction);
    }

   
}