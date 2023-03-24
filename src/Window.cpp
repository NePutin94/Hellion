//
// Created by NePutin on 1/28/2023.
//

#include "../include/Window.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

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

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            vulkanHelper.drawFrame(window, draw_data);
        }
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