//
// Created by NePutin on 3/23/2023.
//

#include "../../include/vulkan/HSwapChain.h"

vk::Result Hellion::HSwapChain::submitCommandBuffers(const vk::CommandBuffer& commandBuffers, uint32_t imageIndex)
{
    HELLION_ZONE_PROFILING()
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

    device.getDevice().resetFences(1, &inFlightFences[currentFrame]);

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

vk::SurfaceFormatKHR Hellion::HSwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    if(availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
        return {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};

    for(const auto& availableFormat: availableFormats)
        if(availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;

    return availableFormats[0];
}

vk::PresentModeKHR Hellion::HSwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

    for(const auto& availablePresentMode: availablePresentModes)
    {
        if(availablePresentMode == vk::PresentModeKHR::eMailbox)
            return availablePresentMode;
        else if(availablePresentMode == vk::PresentModeKHR::eImmediate)
            bestMode = availablePresentMode;
    }

    return bestMode;
}

vk::Extent2D Hellion::HSwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    } else
    {
        vk::Extent2D actualExtent = windowExtent;
        actualExtent.width = std::max(
                capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
                capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

void Hellion::HSwapChain::createSwapChain()
{
    HELLION_ZONE_PROFILING()
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

    createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

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

void Hellion::HSwapChain::createImageViews()
{
    HELLION_ZONE_PROFILING()
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

vk::Format Hellion::HSwapChain::findDepthFormat()
{
    return device.findSupportedFormat(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void Hellion::HSwapChain::createRenderPass()
{
    HELLION_ZONE_PROFILING()
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

void Hellion::HSwapChain::createDepthResources()
{
    HELLION_ZONE_PROFILING()
    vk::Format depthFormat = findDepthFormat();
    swapChainDepthFormat = depthFormat;
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

void Hellion::HSwapChain::createFramebuffers()
{
    HELLION_ZONE_PROFILING()
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

void Hellion::HSwapChain::createSyncObjects()
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

vk::ResultValue<uint32_t> Hellion::HSwapChain::acquireNextImage()
{
    HELLION_ZONE_PROFILING()
    device.getDevice().waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    try
    {
        vk::ResultValue result = device.getDevice().acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(),
                                                                        imageAvailableSemaphores[currentFrame], nullptr);
        return result;
    }
    catch (vk::OutOfDateKHRError& err)
    {
        return vk::ResultValue<uint32_t>{vk::Result::eErrorOutOfDateKHR, 0};
    } catch (vk::SystemError& err)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
}

void Hellion::HSwapChain::init()
{
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

void Hellion::HSwapChain::cleanup()
{
    for(auto imageView: swapChainImageViews)
        device.getDevice().destroyImageView(imageView);

    swapChainImageViews.clear();

    if(swapChain != nullptr)
    {
        device.getDevice().destroySwapchainKHR(swapChain);
        swapChain = nullptr;
    }

    for(int i = 0; i < depthImages.size(); i++)
    {
        vmaDestroyImage(device.getAllocator(), depthImages[i], depthImageAllocs[i]);
        device.getDevice().destroy(depthImageViews[i]);
    }

    for(auto framebuffer: swapChainFramebuffers)
        device.getDevice().destroyFramebuffer(framebuffer);

    device.getDevice().destroy(renderPass);
}

void Hellion::HSwapChain::cleanupSyncObjects()
{
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        device.getDevice().destroySemaphore(renderFinishedSemaphores[i]);
        device.getDevice().destroySemaphore(imageAvailableSemaphores[i]);
        device.getDevice().destroyFence(inFlightFences[i]);
    }
}
