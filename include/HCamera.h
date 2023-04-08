//
// Created by NePutin on 4/6/2023.
//

#ifndef HELLION_HCAMERA_H
#define HELLION_HCAMERA_H

#include <glm/common.hpp>
#include "glm/vec3.hpp"
#include "glm/detail/type_mat4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "GLFW/glfw3.h"

namespace Hellion
{


    class HCamera
    {
    public:
        HCamera(glm::vec2 size) : size(size)
        {

        }

        glm::mat4 getViewMatrix()
        {
            return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }

        glm::mat4 getProjectionMatrix()
        {
            return glm::perspective(glm::radians(fov), (float) size.x / (float) size.y, 0.1f, 100.0f);
        }

        void update(GLFWwindow* window)
        {
            if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraPos += cameraSpeed * cameraFront;
            if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                cameraPos.z += cameraSpeed;
            if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
                cameraPos.z -= cameraSpeed;
            if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraPos -= cameraSpeed * cameraFront;
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        }

    public:
        float fov = 60.f;
        float yaw = -90;
        float pitch = 0;
        float cameraSpeed = 0.005f;

        glm::vec2 size;
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 direction;
        glm::vec2 mousePos;
    };

} // Hellion

#endif //HELLION_HCAMERA_H
