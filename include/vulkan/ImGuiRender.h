//
// Created by NePutin on 3/18/2023.
//

#ifndef HELLION_IMGUIRENDER_H
#define HELLION_IMGUIRENDER_H

#include "HTexture.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

namespace Hellion
{
    class ImGuiRenderer
    {
    public:
        ImGuiRenderer() = default;
        void initImgui(vk::Device& device, GLFWwindow* window, vk::Instance& instance, vk::PhysicalDevice& pdevice, vk::Queue& gqueue, vk::RenderPass& renderPass,
                       vk::CommandPool& commandPool)
        {
            std::array<vk::DescriptorPoolSize, 11> pool_sizes =
                    {
                            vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic, 1000},
                            vk::DescriptorPoolSize{vk::DescriptorType::eInputAttachment, 1000}
                    };

            vk::DescriptorPoolCreateInfo poolInfo
                    {
                            vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                            1000 * pool_sizes.size(),
                            pool_sizes.size(),
                            pool_sizes.data(),
                    };
            auto imguiPool = device.createDescriptorPool(poolInfo);

            ImGui::CreateContext();

            ImGui_ImplGlfw_InitForVulkan(window, true);

            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = instance;
            init_info.PhysicalDevice = pdevice;
            init_info.Device = device;
            init_info.Queue = gqueue;
            init_info.DescriptorPool = imguiPool;
            init_info.MinImageCount = 3;
            init_info.ImageCount = 3;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

            ImGui_ImplVulkan_Init(&init_info, renderPass);

            //font load
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;
            vk::CommandBuffer commandBuffer;
            commandBuffer = device.allocateCommandBuffers(allocInfo)[0];
            vk::CommandBufferBeginInfo beginInfo{};
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

            commandBuffer.begin(beginInfo);

            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

            commandBuffer.end();

            vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            gqueue.submit(submitInfo);
            gqueue.waitIdle();
            device.freeCommandBuffers(commandPool, commandBuffer);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }

    };
}

#endif //HELLION_IMGUIRENDER_H
