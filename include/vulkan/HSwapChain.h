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
    private:
        void init()
        {
            createSwapChain();
            createImageViews();
            createRenderPass();
            createDepthResources();
            createFramebuffers();
            createSyncObjects();
        }

    public:
        HSwapChain(HWindow &window, HDevice &device) : window{window}, device{device} { init(); }
        vk::Extent2D& getSwapChainExtent()
        {
            return swapChainExtent;
        }

        vk::RenderPass& getRenderPass()
        {
            return renderPass;
        }

        uint32_t acquireNextImage() {
            device.getDevice().waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

            vk::ResultValue result = device.getDevice().acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(),
                                       imageAvailableSemaphores[currentFrame], nullptr);
            return  result.value;
        }

        vk::Result submitCommandBuffers(const vk::CommandBuffer& commandBuffers, uint32_t imageIndex) {

            vk::SubmitInfo submitInfo = {};

            vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
            vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffers;

            vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            device.getGraphicsQueue().submit(submitInfo, inFlightFences[currentFrame]);

            vk::PresentInfoKHR presentInfo = {};
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            vk::SwapchainKHR swapChains[] = {swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imageIndex;

            vk::Result resultPresent;
            try
            {
                resultPresent = device.getPresentQueue().presentKHR(presentInfo);
            } catch (vk::OutOfDateKHRError err)
            {
                resultPresent = vk::Result::eErrorOutOfDateKHR;
            } catch (vk::SystemError err)
            {
                throw std::runtime_error("failed to present swap chain image!");
            }

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

            return resultPresent;
        }
    private:
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
        {
            if(availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
            {
                return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
            }

            for(const auto& availableFormat: availableFormats)
            {
                if(availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
        {
            vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

            for(const auto& availablePresentMode: availablePresentModes)
            {
                if(availablePresentMode == vk::PresentModeKHR::eMailbox)
                {
                    return availablePresentMode;
                } else if(availablePresentMode == vk::PresentModeKHR::eImmediate)
                {
                    bestMode = availablePresentMode;
                }
            }

            return bestMode;
        }

        vk::Extent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
        {
            if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            } else
            {
                VkExtent2D actualExtent = {
                        static_cast<uint32_t>(window.getWidth()),
                        static_cast<uint32_t>(window.getHeight())};

                actualExtent.width = std::max(
                        capabilities.minImageExtent.width,
                        std::min(capabilities.maxImageExtent.width, actualExtent.width));
                actualExtent.height = std::max(
                        capabilities.minImageExtent.height,
                        std::min(capabilities.maxImageExtent.height, actualExtent.height));

                return actualExtent;
            }
        }

        void createSwapChain()
        {
            SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

            vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
            vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
                imageCount = swapChainSupport.capabilities.maxImageCount;

            vk::SwapchainCreateInfoKHR createInfo(
                    vk::SwapchainCreateFlagsKHR(),
                    device.getSurface(),
                    imageCount,
                    surfaceFormat.format,
                    surfaceFormat.colorSpace,
                    extent,
                    1, // imageArrayLayers
                    vk::ImageUsageFlagBits::eColorAttachment
            );

            QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
            uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

            if(indices.graphicsFamily != indices.presentFamily)
            {
                createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            } else
            {
                createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

            try
            {
                swapChain = device.getDevice().createSwapchainKHR(createInfo);
            }
            catch (vk::SystemError err)
            {
                throw std::runtime_error("failed to create swap chain!");
            }

            swapChainImages = device.getDevice().getSwapchainImagesKHR(swapChain);

            swapChainImageFormat = surfaceFormat.format;
            swapChainExtent = extent;
        }

        void createImageViews()
        {
            swapChainImageViews.resize(swapChainImages.size());
            for(size_t i = 0; i < swapChainImages.size(); i++)
            {
                try
                {
                    swapChainImageViews[i] = device.createImageView(
                            swapChainImages[i],
                            swapChainImageFormat,
                            vk::ImageAspectFlagBits::eColor,
                            1,  // mip levels
                            vk::ImageViewType::e2D);
                }
                catch (vk::SystemError err)
                {
                    throw std::runtime_error("failed to create image views!");
                }
            }
        }

        vk::Format findDepthFormat()
        {
            return device.findSupportedFormat(
                    {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                    vk::ImageTiling::eOptimal,
                    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        }

        void createRenderPass()
        {
            vk::AttachmentDescription colorAttachment = {};
            colorAttachment.format = swapChainImageFormat;
            colorAttachment.samples = vk::SampleCountFlagBits::e1;
            colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
            colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
            colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

            vk::AttachmentDescription depthAttachment{};
            depthAttachment.format = findDepthFormat();
            depthAttachment.samples = vk::SampleCountFlagBits::e1;
            depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
            depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
            depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            vk::AttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

            vk::AttachmentReference depthAttachmentRef{};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            vk::SubpassDescription subpass = {};
            subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            vk::SubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
            dependency.srcAccessMask = vk::AccessFlags();
            dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
            dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

            std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
            vk::RenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;

            try
            {
                renderPass = device.getDevice().createRenderPass(renderPassInfo);
            } catch (vk::SystemError err)
            {
                throw std::runtime_error("failed to create render pass!");
            }
        }

        size_t imageCount()
        { return swapChainImages.size(); }

        void createDepthResources()
        {
            vk::Format depthFormat = findDepthFormat();

            depthImages.resize(imageCount());
            depthImageAllocs.resize(imageCount());
            depthImageViews.resize(imageCount());

            for(int i = 0; i < depthImages.size(); i++)
            {
                auto [_depthImage, _depthImageAlloc] = device.createImage(swapChainExtent.width, swapChainExtent.height, 1, depthFormat,
                                                                          vk::ImageTiling::eOptimal,
                                                                          vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageCreateFlags{}, 1);
                depthImages[i] = _depthImage;
                depthImageAllocs[i] = _depthImageAlloc;
                depthImageViews[i] = device.createImageView(depthImages[i], depthFormat, vk::ImageAspectFlagBits::eDepth, 1, vk::ImageViewType::e2D);
            }
        }

        void createFramebuffers()
        {
            swapChainFramebuffers.resize(swapChainImageViews.size());

            for(size_t i = 0; i < swapChainImageViews.size(); i++)
            {
                std::array<vk::ImageView, 2> attachments = {
                        swapChainImageViews[i],
                        depthImageViews[i]
                };

                vk::FramebufferCreateInfo framebufferInfo = {};
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;

                try
                {
                    swapChainFramebuffers[i] = device.getDevice().createFramebuffer(framebufferInfo);
                } catch (vk::SystemError err)
                {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
        }

        void createSyncObjects()
        {
            imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

            try
            {
                for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                {
                    vk::SemaphoreCreateInfo semaphoreInfo{};

                    vk::FenceCreateInfo fenceInfo{};
                    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

                    imageAvailableSemaphores[i] = device.getDevice().createSemaphore(semaphoreInfo);
                    renderFinishedSemaphores[i] = device.getDevice().createSemaphore({});
                    inFlightFences[i] = device.getDevice().createFence(fenceInfo);
                }
            } catch (vk::SystemError err)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    };

} // Hellion

#endif //HELLION_HSWAPCHAIN_H
