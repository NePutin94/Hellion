//
// Created by NePutin on 2/19/2023.
//

#include "../../include/vulkan/HDevice.h"

void Hellion::HDevice::init()
{
    createInstance();
    setupDebug();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createVmaAllocator();
    createCommandPool();
}

bool Hellion::HDevice::supported(std::vector<const char*>& extensions, const std::vector<const char*>& layers, bool debug)
{
    std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

    if(debug)
    {
        fmt::println("Device can support the following extensions:");
        for(vk::ExtensionProperties supportedExtension: supportedExtensions)
        {
            fmt::println("{}", supportedExtension.extensionName);
        }
    }

    bool found;
    for(const char* extension: extensions)
    {
        found = false;
        for(vk::ExtensionProperties supportedExtension: supportedExtensions)
        {
            if(strcmp(extension, supportedExtension.extensionName) == 0)
            {
                found = true;
                if(debug)
                {
                    fmt::println("Extension {} is supported!", extension);
                }
            }
        }
        if(!found)
        {
            if(debug)
            {
                fmt::println("Extension {} is not supported!", extension);
            }
            return false;
        }
    }

    //check layer support
    std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

    if(debug)
    {
        fmt::println("Device can support the following layers");
        for(vk::LayerProperties supportedLayer: supportedLayers)
        {
            fmt::println("{}", supportedLayer.layerName);
        }
    }

    for(const char* layer: layers)
    {
        found = false;
        for(vk::LayerProperties supportedLayer: supportedLayers)
        {
            if(strcmp(layer, supportedLayer.layerName) == 0)
            {
                found = true;
                if(debug)
                {
                    fmt::println("Layer {} is supported!", layer);
                }
            }
        }
        if(!found)
        {
            if(debug)
            {
                fmt::println("Layer {} is not supported!", layer);
            }
            return false;
        }
    }

    return true;
}

std::vector<const char*> Hellion::HDevice::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void Hellion::HDevice::createInstance()
{
    vk::ApplicationInfo appInfo = vk::ApplicationInfo("Test", VK_MAKE_API_VERSION(0, 1, 0, 0), "No engine", VK_MAKE_API_VERSION(0, 1, 0, 0),
                                                      VK_API_VERSION_1_3);
    auto extensions = getRequiredExtensions();
    if(!supported(extensions, validationLayers, false))
    {
        //error
    }

    vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
            vk::InstanceCreateFlags(),
            &appInfo,
            0, nullptr,
            static_cast<uint32_t>(extensions.size()), extensions.data()
    );
    if(enableValidationLayers)
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo3 = vk::DebugUtilsMessengerCreateInfoEXT(
                vk::DebugUtilsMessengerCreateFlagsEXT(),
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                debugCallback
        );
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        createInfo.pNext = &createInfo3;
    }

    instance = vk::createInstance(createInfo);
    dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
}

void Hellion::HDevice::setupDebug()
{
    if(!enableValidationLayers) return;

    auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            debugCallback,
            nullptr
    );

    debugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
}

void Hellion::HDevice::pickPhysicalDevice()
{
    std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
    if(physicalDevices.empty())
        fmt::println("failed to find GPUs with Vulkan support!");

    for(const auto& device: physicalDevices)
    {
        if(isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }
}

Hellion::QueueFamilyIndices Hellion::HDevice::findQueueFamilies(const vk::PhysicalDevice& device)
{
    QueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for(const auto& queueFamily: queueFamilies)
    {
        if(queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if(queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, surface))
            indices.presentFamily = i;

        if(indices.isComplete())
            break;
        i++;
    }

    return indices;
}

bool Hellion::HDevice::checkDeviceExtensionSupport(const vk::PhysicalDevice& device)
{
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto& extension: device.enumerateDeviceExtensionProperties())
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

Hellion::SwapChainSupportDetails Hellion::HDevice::querySwapChainSupport(const vk::PhysicalDevice& device)
{
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.formats = device.getSurfaceFormatsKHR(surface);
    details.presentModes = device.getSurfacePresentModesKHR(surface);

    return details;
}

bool Hellion::HDevice::isDeviceSuitable(const vk::PhysicalDevice& device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if(extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    auto supportedFeatures = device.getFeatures();

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void Hellion::HDevice::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;

    for(uint32_t queueFamily: uniqueQueueFamilies)
        queueCreateInfos.push_back({vk::DeviceQueueCreateFlags(), queueFamily, 1, &queuePriority});

    auto deviceFeatures = vk::PhysicalDeviceFeatures();
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    auto createInfo = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), static_cast<uint32_t>(queueCreateInfos.size()), queueCreateInfos.data());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if(enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    try
    {
        device = physicalDevice.createDevice(createInfo);
    }
    catch (vk::SystemError& err)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

void Hellion::HDevice::createVmaAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.vulkanApiVersion = GetVulkanApiVersion();

    vmaCreateAllocator(&allocatorInfo, &g_hAllocator);
}

void Hellion::HDevice::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    try
    {
        commandPool = device.createCommandPool(poolInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

std::pair<VmaAllocation, VmaAllocationInfo>
Hellion::HDevice::createBufferVma(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::Buffer& buffer, VmaAllocationCreateFlags flags)
{
    vk::BufferCreateInfo bufferInfo = {};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo vbAllocCreateInfo = {};
    vbAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vbAllocCreateInfo.flags = flags;

    VmaAllocation stagingVertexBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingVertexBufferAllocInfo = {};
    vmaCreateBuffer(g_hAllocator, (VkBufferCreateInfo*) &bufferInfo, &vbAllocCreateInfo, (VkBuffer*) &buffer, &stagingVertexBufferAlloc,
                    &stagingVertexBufferAllocInfo);
    return {stagingVertexBufferAlloc, stagingVertexBufferAllocInfo};
}

vk::CommandBuffer Hellion::HDevice::beginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void Hellion::HDevice::endSingleTimeCommands(vk::CommandBuffer& commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue.submit(submitInfo);

    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, commandBuffer);
}

std::pair<vk::Image, VmaAllocation>
Hellion::HDevice::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                              vk::ImageCreateFlags flags, uint32_t arrayLayers)
{
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arrayLayers;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = usage;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.flags = flags;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocCreateInfo.priority = 1.0f;

    vk::Image img;
    VmaAllocation alloc;
    vmaCreateImage(g_hAllocator, reinterpret_cast<VkImageCreateInfo*>(&imageInfo), &allocCreateInfo, reinterpret_cast<VkImage*>(&img), &alloc, nullptr);
    return {img, alloc};
}

void Hellion::HDevice::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels,
                                             uint32_t layerCount)
{
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if(newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

        if(hasStencilComponent(format))
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eColor;
    } else
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;

    if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

    endSingleTimeCommands(commandBuffer);
}

void Hellion::HDevice::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{
            width,
            height,
            1
    };
    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
    endSingleTimeCommands(commandBuffer);
}

vk::ImageView
Hellion::HDevice::createImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels, vk::ImageViewType viewType)
{
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = image;
    viewInfo.format = format;
    viewInfo.viewType = viewType;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = viewType == vk::ImageViewType::eCube ? 6 : 1;
    vk::ImageView imageView = device.createImageView(viewInfo);
    return imageView;
}

vk::Format Hellion::HDevice::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for(vk::Format format: candidates)
    {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if(tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
        {
            return format;
        } else if(tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

void Hellion::HDevice::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

    commandBuffer.end();

    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, commandBuffer);
}

void Hellion::HDevice::cleanup()
{
    device.destroyCommandPool(commandPool);
    vmaDestroyAllocator(g_hAllocator);
    device.destroy();
    if(enableValidationLayers)
        instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
    instance.destroySurfaceKHR(surface);
    instance.destroy();
}
