//
// Created by NePutin on 3/23/2023.
//

#include "../../include/vulkan/HWindow.h"


void Hellion::HWindow::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Hellion::HWindow::createWindowSurface(vk::Instance instance, vk::SurfaceKHR* surface)
{
    if(glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(surface)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}
