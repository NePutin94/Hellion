//
// Created by NePutin on 3/23/2023.
//

#ifndef HELLION_HSWAPCHAIN_H
#define HELLION_HSWAPCHAIN_H

#include <vulkan/vulkan.hpp>
#include "HWindow.h"
#include "HDevice.h"

namespace Hellion
{
    class HSwapChain
    {
    private:
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;

        std::vector<vk::Framebuffer> swapChainFramebuffers;
        vk::RenderPass renderPass;

        std::vector<vk::Image> depthImages;
        std::vector<vk::ImageView> depthImageViews;
        std::vector<VmaAllocation> depthImageAllocs;

        std::vector<vk::Image> swapChainImages;
        std::vector<vk::ImageView> swapChainImageViews;

        HWindow& window;
        HDevice& device;

        VkSwapchainKHR swapChain;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;
        size_t currentFrame = 0;
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    public:
        HSwapChain(HWindow& window, HDevice& device) : window{window}, device{device}
        { init(); }

        HSwapChain(const HSwapChain&) = delete;

        void operator=(const HSwapChain&) = delete;

        void cleanupSyncObjects();

        ~HSwapChain()
        {
            cleanup();
            cleanupSyncObjects();
        }

        vk::Framebuffer getFrameBuffer(int index)
        { return swapChainFramebuffers[index]; }

        vk::RenderPass getRenderPass()
        { return renderPass; }

        vk::ImageView getImageView(int index)
        { return swapChainImageViews[index]; }

        vk::Extent2D& getSwapChainExtent()
        { return swapChainExtent; }

        vk::Format getSwapChainImageFormat()
        { return swapChainImageFormat; }

        size_t imageCount()
        { return swapChainImages.size(); }

        uint32_t acquireNextImage();

        vk::Result submitCommandBuffers(const vk::CommandBuffer& commandBuffers, uint32_t imageIndex);

    private:
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

        vk::Extent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        void createSwapChain();

        void createImageViews();

        vk::Format findDepthFormat();

        void createRenderPass();

        void createDepthResources();

        void createFramebuffers();

        void createSyncObjects();

        void init();

        void cleanup();
    };

} // Hellion

#endif //HELLION_HSWAPCHAIN_H
