//
// Created by NePutin on 1/28/2023.
//
#include "../include/Window.h"

void Hellion::Window::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(windowSize.x, windowSize.y, "Vulkan", nullptr, nullptr);
}

void Hellion::Window::mainLoop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        vulkanHelper.drawFrame();
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