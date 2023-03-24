//
// Created by NePutin on 3/23/2023.
//

#ifndef HELLION_HELLWINDOW_H
#define HELLION_HELLWINDOW_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace Hellion
{
    class HWindow
    {
    private:
        int width;
        int height;
        GLFWwindow* window;
        std::string windowName;
        bool framebufferResized = false;
    public:
        HWindow(int width, int height) : HWindow(width, height, "Vulkan Application")
        {}

        HWindow(int width, int height, std::string windowName) : width{width}, height{height}, windowName{windowName}
        {
            initWindow();
        }

        void createWindowSurface(vk::Instance instance, vk::SurfaceKHR* surface)
        {
            if(glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(surface)) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create window surface!");
            }
        }

        ~HWindow()
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        int getWidth() const
        {
            return width;
        }

        void setWidth(int width)
        {
            width = width;
        }

        int getHeight() const
        {
            return height;
        }

        void setHeight(int height)
        {
            height = height;
        }

        GLFWwindow* getWindow() const
        {
            return window;
        }

        void setWindow(GLFWwindow* window)
        {
            this->window = window;
        }

    private:
        void initWindow()
        {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        }

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
        {
            auto app = reinterpret_cast<HWindow*>(glfwGetWindowUserPointer(window));
            app->framebufferResized = true;
            app->width = width;
            app->height = height;
        }
    };

} // Hellion

#endif //HELLION_HELLWINDOW_H
