//
// Created by NePutin on 3/23/2023.
//

#ifndef HELLION_HSWAPCHAIN_H
#define HELLION_HSWAPCHAIN_H

#include <vulkan/vulkan.hpp>
#include "HWindow.h"
#include "HDevice.h"
#include <memory>
#include "../core/Profiling.h"

namespace Hellion
{
    class HSwapChain
    {
    private:
        vk::Format swapChainImageFormat;
        vk::Format swapChainDepthFormat;
        vk::Extent2D swapChainExtent;
        vk::Extent2D windowExtent;

        std::shared_ptr<HSwapChain> oldSwapChain;

        std::vector<vk::Framebuffer> swapChainFramebuffers;
        vk::RenderPass renderPass;

        std::vector<vk::Image> depthImages;
        std::vector<vk::ImageView> depthImageViews;
        std::vector<VmaAllocation> depthImageAllocs;

        std::vector<vk::Image> swapChainImages;
        std::vector<vk::ImageView> swapChainImageViews;

        HDevice& device;

        VkSwapchainKHR swapChain;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;
        size_t currentFrame = 0;
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    public:
        HSwapChain(HDevice& deviceRef, vk::Extent2D windowExtent) : device{deviceRef}, windowExtent{windowExtent}
        { init(); }

        HSwapChain(HDevice& deviceRef, vk::Extent2D extent, std::shared_ptr<HSwapChain> previous) : device{deviceRef}, windowExtent{extent},
                                                                                                    oldSwapChain{previous}
        {
            init();
            oldSwapChain = nullptr;
        }

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

        float extentAspectRatio()
        { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }

        vk::ResultValue<uint32_t> acquireNextImage();

        vk::Result submitCommandBuffers(const vk::CommandBuffer& commandBuffers, uint32_t imageIndex);

        bool compareSwapFormats(const HSwapChain& swapChain) const
        {
            return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
                   swapChain.swapChainImageFormat == swapChainImageFormat;
        }

    private:
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

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
