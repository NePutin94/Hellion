//
// Created by NePutin on 1/28/2023.
//

#ifndef HELLION_WINDOW_H
#define HELLION_WINDOW_H

#include <glm/vec2.hpp>
#include "vulkan/VulkanHelper.h"

namespace Hellion
{
    class Window
    {
    private:
        GLFWwindow* window;
        glm::ivec2 windowSize;
        VulkanHelper vulkanHelper;

    private:
        void initWindow();

        void initVulkan();

        void mainLoop();

        void cleanup();

    public:
        explicit Window(glm::ivec2 windowSize) : windowSize(windowSize)
        {}

        void run()
        {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }
    };
}
#endif //HELLION_WINDOW_H
