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

        ~HWindow()
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        void createWindowSurface(vk::Instance instance, vk::SurfaceKHR* surface);

        bool shouldClose() { return glfwWindowShouldClose(window); }

        int getWidth() const
        { return width; }

        void setWidth(int _width)
        { width = _width; }

        int getHeight() const
        { return height; }

        void setHeight(int _height)
        { height = _height; }

        GLFWwindow* getWindow() const
        { return window; }

        void setWindow(GLFWwindow* _window)
        { window = _window; }

        bool wasWindowResized()
        { return framebufferResized; }

        void resetWindowResizedFlag() { framebufferResized = false; }

        vk::Extent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
    private:
        void initWindow();

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
