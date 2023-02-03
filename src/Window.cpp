//
// Created by NePutin on 1/28/2023.
//
#include "../include/Window.h"

void Hellion::Window::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(windowSize.x, windowSize.y, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, &vulkanHelper);
    glfwSetFramebufferSizeCallback(window, Hellion::VulkanHelper::framebufferResizeCallback);
}

void Hellion::Window::mainLoop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        vulkanHelper.drawFrame(window);
    }
    vulkanHelper.wait();
}

void Hellion::Window::cleanup()
{
    vulkanHelper.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Hellion::Window::initVulkan()
{
    vulkanHelper.init(window);
}