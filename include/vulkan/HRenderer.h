//
// Created by NePutin on 3/25/2023.
//

#ifndef HELLION_HRENDERER_H
#define HELLION_HRENDERER_H

#include <vulkan/vulkan.hpp>
#include "HWindow.h"
#include "HDevice.h"
#include "HSwapChain.h"

namespace Hellion
{

    class HRenderer
    {
    public:
        HRenderer(HWindow& window, HDevice& device);

        ~HRenderer();

        HRenderer(const HRenderer&) = delete;

        HRenderer& operator=(const HRenderer&) = delete;

        vk::RenderPass getSwapChainRenderPass() const
        { return swapChain->getRenderPass(); }

        float getAspectRatio() const
        { return swapChain->extentAspectRatio(); }

        bool isFrameInProgress() const
        { return isFrameStarted; }

        auto* getSwapChain()
        {return swapChain.get();}

        vk::CommandBuffer& getCurrentCommandBuffer()
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        vk::CommandBuffer beginFrame()
        {
            assert(!isFrameStarted && "Can't call beginFrame while already in progress");

            auto result = swapChain->acquireNextImage();
            if(result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                recreateSwapChain();
                return nullptr;
            }

            if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
                throw std::runtime_error("failed to acquire swap chain image!");
            }

            isFrameStarted = true;

            auto commandBuffer = getCurrentCommandBuffer();
            vk::CommandBufferBeginInfo beginInfo{};

            commandBuffer.begin(beginInfo);
            return commandBuffer;
        }

        void endFrame()
        {
            assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
            auto commandBuffer = getCurrentCommandBuffer();
            commandBuffer.end();

            auto result = swapChain->submitCommandBuffers(commandBuffer, currentImageIndex);

            if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR ||
               window.wasWindowResized())
            {
                window.resetWindowResizedFlag();
                recreateSwapChain();
            } else if(result != vk::Result::eSuccess)
            {
                throw std::runtime_error("failed to present swap chain image!");
            }
            isFrameStarted = false;
            currentFrameIndex = (currentFrameIndex + 1) % HSwapChain::MAX_FRAMES_IN_FLIGHT;
        }

        void beginSwapChainRenderPass(vk::CommandBuffer commandBuffer)
        {
            assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
            assert(
                    commandBuffer == getCurrentCommandBuffer() &&
                    "Can't begin render pass on command buffer from a different frame");

            vk::RenderPassBeginInfo renderPassInfo{};
            renderPassInfo.renderPass = swapChain->getRenderPass();
            renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);

            renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
            renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

            std::array<vk::ClearValue, 2> clearValues{};
            clearValues[0].color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
            clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();


            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            vk::Viewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
            viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vk::Rect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
            commandBuffer.setViewport(0, viewport);
            commandBuffer.setScissor(0, scissor);
        }

        void endSwapChainRenderPass(vk::CommandBuffer commandBuffer)
        {
            assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
            assert(
                    commandBuffer == getCurrentCommandBuffer() &&
                    "Can't end render pass on command buffer from a different frame");
            commandBuffer.endRenderPass();
        }

    private:
        void createCommandBuffers()
        {
            commandBuffers.resize(HSwapChain::MAX_FRAMES_IN_FLIGHT);

            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandPool = device.getCommandPool();
            allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

            commandBuffers = device.getDevice().allocateCommandBuffers(allocInfo);
        }

        void freeCommandBuffers()
        {
            device.getDevice().freeCommandBuffers(device.getCommandPool(), commandBuffers);
        }

        void recreateSwapChain()
        {
            auto extent = window.getExtent();
            while (extent.width == 0 || extent.height == 0) {
                extent = window.getExtent();
                glfwWaitEvents();
            }
            vkDeviceWaitIdle(device.getDevice());

            if (swapChain == nullptr) {
                swapChain = std::make_unique<HSwapChain>(device, extent);
            } else {
                std::shared_ptr<HSwapChain> oldSwapChain = std::move(swapChain);
                swapChain = std::make_unique<HSwapChain>(device, extent, oldSwapChain);

                if (!oldSwapChain->compareSwapFormats(*swapChain.get())) {
                    throw std::runtime_error("Swap chain image(or depth) format has changed!");
                }
            }
        }

        HWindow& window;
        HDevice& device;
        std::unique_ptr<HSwapChain> swapChain;
        std::vector<vk::CommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };

} // Hellion

#endif //HELLION_HRENDERER_H
