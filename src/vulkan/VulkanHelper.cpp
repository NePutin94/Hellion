//
// Created by NePutin on 1/28/2023.
//
#include "../../include/vulkan/VulkanHelper.h"

void Hellion::VulkanHelper::createInstance()
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

std::vector<const char*> Hellion::VulkanHelper::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void Hellion::VulkanHelper::setupDebug()
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

bool Hellion::VulkanHelper::supported(std::vector<const char*>& extensions, const std::vector<const char*>& layers, bool debug)
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

void Hellion::VulkanHelper::init(GLFWwindow* window)
{
    createInstance();
    setupDebug();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain(window);
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}

void Hellion::VulkanHelper::cleanup()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        device.destroySemaphore(renderFinishedSemaphores[i]);
        device.destroySemaphore(imageAvailableSemaphores[i]);
        device.destroyFence(inFlightFences[i]);
    }
    device.destroyCommandPool(commandPool);
    for (auto framebuffer : swapChainFramebuffers) {
        device.destroyFramebuffer(framebuffer);
    }
    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyRenderPass(renderPass);
    for(const auto& imageView: swapChainImageViews)
        device.destroyImageView(imageView);
    device.destroySwapchainKHR(swapChain);
    device.destroy();
    instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
    instance.destroySurfaceKHR(surface);
    instance.destroy();
}

void Hellion::VulkanHelper::pickPhysicalDevice()
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

bool Hellion::VulkanHelper::isDeviceSuitable(const vk::PhysicalDevice& device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if(extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

Hellion::VulkanHelper::QueueFamilyIndices Hellion::VulkanHelper::findQueueFamilies(const vk::PhysicalDevice& device)
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

bool
Hellion::VulkanHelper::checkDeviceExtensionSupport(const vk::PhysicalDevice& device)
{
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto& extension: device.enumerateDeviceExtensionProperties())
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Hellion::VulkanHelper::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;

    for(uint32_t queueFamily: uniqueQueueFamilies)
        queueCreateInfos.push_back({vk::DeviceQueueCreateFlags(), queueFamily, 1, &queuePriority});

    auto deviceFeatures = vk::PhysicalDeviceFeatures();
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
    catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

void Hellion::VulkanHelper::createSurface(GLFWwindow* window)
{
    VkSurfaceKHR c_style_surface;
    if(glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface) != VK_SUCCESS)
    {}
    surface = c_style_surface;
}

void Hellion::VulkanHelper::createSwapChain(GLFWwindow* window)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;
    vk::SwapchainCreateInfoKHR createInfo(
            vk::SwapchainCreateFlagsKHR(),
            surface,
            imageCount,
            surfaceFormat.format,
            surfaceFormat.colorSpace,
            extent,
            1, // imageArrayLayers
            vk::ImageUsageFlagBits::eColorAttachment
    );

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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
        swapChain = device.createSwapchainKHR(createInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    swapChainImages = device.getSwapchainImagesKHR(swapChain);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

Hellion::VulkanHelper::SwapChainSupportDetails Hellion::VulkanHelper::querySwapChainSupport(const vk::PhysicalDevice& device)
{
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.formats = device.getSurfaceFormatsKHR(surface);
    details.presentModes = device.getSurfacePresentModesKHR(surface);

    return details;
}

vk::SurfaceFormatKHR Hellion::VulkanHelper::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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

vk::PresentModeKHR Hellion::VulkanHelper::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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

vk::Extent2D Hellion::VulkanHelper::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    } else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Hellion::VulkanHelper::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());
    for(size_t i = 0; i < swapChainImages.size(); i++)
    {
        vk::ImageViewCreateInfo createInfo = {};
        createInfo.image = swapChainImages[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        try
        {
            swapChainImageViews[i] = device.createImageView(createInfo);
        }
        catch (vk::SystemError err)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void Hellion::VulkanHelper::createGraphicsPipeline()
{
    auto vertShaderCode = readFile("../Data/Shaders/vert.spv");
    auto fragShaderCode = readFile("../Data/Shaders/frag.spv");

    auto vertShaderModule = createShaderModule(vertShaderCode);
    auto fragShaderModule = createShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
            {
                    vk::PipelineShaderStageCreateFlags(),
                    vk::ShaderStageFlagBits::eVertex,
                    *vertShaderModule,
                    "main"
            },
            {
                    vk::PipelineShaderStageCreateFlags(),
                    vk::ShaderStageFlagBits::eFragment,
                    *fragShaderModule,
                    "main"
            }
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::Viewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = swapChainExtent;

    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    try
    {
        pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;

    try
    {
        graphicsPipeline = device.createGraphicsPipeline(nullptr, pipelineInfo).value;
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

vk::UniqueShaderModule Hellion::VulkanHelper::createShaderModule(const std::vector<char>& code)
{
    try
    {
        return device.createShaderModuleUnique({vk::ShaderModuleCreateFlags(), code.size(), reinterpret_cast<const uint32_t*>(code.data())});
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create shader module!");
    }
}

void Hellion::VulkanHelper::createRenderPass()
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

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    try
    {
        renderPass = device.createRenderPass(renderPassInfo);
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Hellion::VulkanHelper::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for(size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        vk::ImageView attachments[] = {
                swapChainImageViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        try
        {
            swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
        } catch (vk::SystemError err)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Hellion::VulkanHelper::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    try {
        commandPool = device.createCommandPool(poolInfo);
    }
    catch (vk::SystemError err) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void Hellion::VulkanHelper::createCommandBuffers()
{
    commandBuffers.resize(swapChainFramebuffers.size());

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    try {
        commandBuffers = device.allocateCommandBuffers(allocInfo);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        vk::CommandBufferBeginInfo beginInfo = {};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

        try {
            commandBuffers[i].begin(beginInfo);
        }
        catch (vk::SystemError err) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        vk::RenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        vk::ClearValue clearColor = { std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        commandBuffers[i].draw(3, 1, 0, 0);

        commandBuffers[i].endRenderPass();

        try {
            commandBuffers[i].end();
        } catch (vk::SystemError err) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void Hellion::VulkanHelper::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    try {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores[i] = device.createSemaphore({});
            renderFinishedSemaphores[i] = device.createSemaphore({});
            inFlightFences[i] = device.createFence({vk::FenceCreateFlagBits::eSignaled});
        }
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

void Hellion::VulkanHelper::drawFrame()
{
    device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    device.resetFences(1, &inFlightFences[currentFrame]);

    uint32_t imageIndex = device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(),
                                                      imageAvailableSemaphores[currentFrame], nullptr).value;

    vk::SubmitInfo submitInfo = {};

    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    try {
        graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    presentQueue.presentKHR(presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
