//
// Created by NePutin on 3/25/2023.
//

#include "../../include/vulkan/HRenderer.h"

void Hellion::HRenderer::createCommandBuffers()
{
    commandBuffers.resize(HSwapChain::MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    commandBuffers = device.getDevice().allocateCommandBuffers(allocInfo);

    for(auto& buff: commandBuffers)
    {
        auto p1 = device.getDldi().vkGetPhysicalDeviceCalibrateableTimeDomainsEXT;
        auto p2 = device.getDldi().vkGetCalibratedTimestampsEXT;
        auto c = TracyVkContextCalibrated(device.getPhysicalDevice(), device.getDevice(), device.getGraphicsQueue(), buff,
                                          p1, p2)
        vkTracyContext.emplace_back(c);
    }
}
