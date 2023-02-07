//
// Created by NePutin on 1/28/2023.
//
#include "../../include/vulkan/VulkanHelper.h"

#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION

#include <vk_mem_alloc.h>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

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
    createVmaAllocator();
    createSwapChain(window);
    createImageViews();
    createRenderPass();

    createDescriptorSetLayout();

    createGraphicsPipeline();
    createCommandPool();

    createDepthResources();

    createFramebuffers();

    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    createVertexBufferVma();
    createIndexBufferVma();

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    createCommandBuffers();
    createSyncObjects();
}

void Hellion::VulkanHelper::cleanup()
{
    char* str;
    vmaBuildStatsString(g_hAllocator, &str, true);
    std::string s = str;
    std::ofstream outfile("log.json");
    outfile << s;
    outfile.close();

    cleanupSwapChain();

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        device.destroySemaphore(renderFinishedSemaphores[i]);
        device.destroySemaphore(imageAvailableSemaphores[i]);
        device.destroyFence(inFlightFences[i]);
    }

    for(int i = 0; i < uniformBuffers.size(); ++i)
        vmaDestroyBuffer(g_hAllocator, uniformBuffers[i], uniformBuffersAllocs[i]);
    vmaDestroyBuffer(g_hAllocator, indexBuffer, indexAllocation);
    vmaDestroyBuffer(g_hAllocator, vertexBuffer, vertexAllocation);

    device.destroySampler(textureSampler);
    device.destroy(textureImageView);
    vmaDestroyImage(g_hAllocator, textureImage, textureImageAlloc);

    device.destroy(graphicsPipeline);
    device.destroy(pipelineLayout);
    device.destroy(renderPass);

    device.destroyDescriptorPool(descriptorPool);
    device.destroyDescriptorSetLayout(descriptorSetLayout);

    device.destroyCommandPool(commandPool);

    vmaDestroyAllocator(g_hAllocator);

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

    auto supportedFeatures = device.getFeatures();

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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
    vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

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

    std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

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
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
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
        std::array<vk::ImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
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

void Hellion::VulkanHelper::createCommandBuffers()
{
    commandBuffers.resize(swapChainFramebuffers.size());

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    try
    {
        commandBuffers = device.allocateCommandBuffers(allocInfo);
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

//    for(size_t i = 0; i < commandBuffers.size(); i++)
//    {
//        vk::CommandBufferBeginInfo beginInfo = {};
//        beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
//
//        try
//        {
//            commandBuffers[i].begin(beginInfo);
//        }
//        catch (vk::SystemError err)
//        {
//            throw std::runtime_error("failed to begin recording command buffer!");
//        }
//
//        vk::RenderPassBeginInfo renderPassInfo = {};
//        renderPassInfo.renderPass = renderPass;
//        renderPassInfo.framebuffer = swapChainFramebuffers[i];
//        renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
//        renderPassInfo.renderArea.extent = swapChainExtent;
//
//        vk::ClearValue clearColor = {std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
//        renderPassInfo.clearValueCount = 1;
//        renderPassInfo.pClearValues = &clearColor;
//
//        commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
//
//        commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
//
//        vk::Buffer vertexBuffers[] = {vertexBuffer};
//
//        vk::DeviceSize offsets[] = {0};
//
//        commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
//
//        commandBuffers[i].bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);
//
//        commandBuffers[i].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
//
//        commandBuffers[i].endRenderPass();
//
//        try
//        {
//            commandBuffers[i].end();
//        } catch (vk::SystemError err)
//        {
//            throw std::runtime_error("failed to record command buffer!");
//        }
//    }
}

void Hellion::VulkanHelper::createSyncObjects()
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

            imageAvailableSemaphores[i] = device.createSemaphore(semaphoreInfo);
            renderFinishedSemaphores[i] = device.createSemaphore({});
            inFlightFences[i] = device.createFence(fenceInfo);
        }
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

void Hellion::VulkanHelper::drawFrame(GLFWwindow* window)
{
    device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    try
    {
        vk::ResultValue result = device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(),
                                                            imageAvailableSemaphores[currentFrame], nullptr);
        imageIndex = result.value;
    } catch (vk::OutOfDateKHRError err)
    {
        recreateSwapChain(window);
        return;
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    device.resetFences(1, &inFlightFences[currentFrame]);

    updateUniformBuffer(currentFrame);

    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    vk::SubmitInfo submitInfo = {};

    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    try
    {
        graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

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
        resultPresent = presentQueue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError err)
    {
        resultPresent = vk::Result::eErrorOutOfDateKHR;
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    if(resultPresent == vk::Result::eSuboptimalKHR || resultPresent == vk::Result::eSuboptimalKHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain(window);
        return;
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Hellion::VulkanHelper::recreateSwapChain(GLFWwindow* window)
{
    int width = 0, height = 0;
    while(width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    device.waitIdle();

    cleanupSwapChain();

    createSwapChain(window);
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void Hellion::VulkanHelper::cleanupSwapChain()
{
    vmaDestroyImage(g_hAllocator,depthImage,depthImageAlloc);
    device.destroy(depthImageView);

    for(auto framebuffer: swapChainFramebuffers)
        device.destroyFramebuffer(framebuffer);

    for(auto imageView: swapChainImageViews)
        device.destroyImageView(imageView);

    device.destroySwapchainKHR(swapChain);
}

uint32_t Hellion::VulkanHelper::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void Hellion::VulkanHelper::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
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

void Hellion::VulkanHelper::createVmaAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.vulkanApiVersion = GetVulkanApiVersion();

    vmaCreateAllocator(&allocatorInfo, &g_hAllocator);
}

std::pair<VmaAllocation, VmaAllocationInfo>
Hellion::VulkanHelper::createBufferVma(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::Buffer& buffer, VmaAllocationCreateFlags flags)
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

void Hellion::VulkanHelper::createVertexBufferVma()
{
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    vk::Buffer stagingBuffer;
    auto [stagingVertexBufferAlloc, stagingVertexBufferAllocInfo] = createBufferVma(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, stagingBuffer,
                                                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);

    memcpy(stagingVertexBufferAllocInfo.pMappedData, vertices.data(), (size_t) bufferSize);

    auto [VertexBufferAlloc, VertexBufferAllocInfo] = createBufferVma(bufferSize,
                                                                      vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                                                                      vertexBuffer, 0);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    vertexAllocation = VertexBufferAlloc;
    vmaDestroyBuffer(g_hAllocator, stagingBuffer, stagingVertexBufferAlloc);
}

void Hellion::VulkanHelper::createIndexBufferVma()
{
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    vk::Buffer stagingBuffer;
    auto [stagingVertexBufferAlloc, stagingVertexBufferAllocInfo] = createBufferVma(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, stagingBuffer,
                                                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);


    memcpy(stagingVertexBufferAllocInfo.pMappedData, indices.data(), (size_t) bufferSize);


    auto [IndexBufferAlloc, IndexBufferAllocInfo] = createBufferVma(bufferSize,
                                                                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                                                                    indexBuffer, 0);
    indexAllocation = IndexBufferAlloc;
    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vmaDestroyBuffer(g_hAllocator, stagingBuffer, stagingVertexBufferAlloc);
}

void Hellion::VulkanHelper::createUniformBuffers()
{
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersAllocs.resize(MAX_FRAMES_IN_FLIGHT);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {

        auto [UniformBufferAlloc, UniformBufferAllocInfo] = createBufferVma(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, uniformBuffers[i],
                                                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                            VMA_ALLOCATION_CREATE_MAPPED_BIT);
        uniformBuffersAllocs[i] = UniformBufferAlloc;
        uniformBuffersMapped[i] = UniformBufferAllocInfo.pMappedData;
    }
}

void Hellion::VulkanHelper::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.bindingCount = (std::uint32_t) bindings.size();
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
}

void Hellion::VulkanHelper::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(15.0f) * time, glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    ubo.time = time * 15;
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Hellion::VulkanHelper::createDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    descriptorPool = device.createDescriptorPool(poolInfo);
}

void Hellion::VulkanHelper::createDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    descriptorSets = device.allocateDescriptorSets(allocInfo);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        device.updateDescriptorSets(descriptorWrites, {});
    }
}

void Hellion::VulkanHelper::recordCommandBuffer(vk::CommandBuffer& buffer, uint32_t imageIndex)
{
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    try
    {
        buffer.begin(beginInfo);
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    buffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = swapChainExtent;
    buffer.setScissor(0, scissor);

    vk::Buffer vertexBuffers[] = {vertexBuffer};

    vk::DeviceSize offsets[] = {0};

    buffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

    buffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);

    buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    buffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    buffer.endRenderPass();

    try
    {
        buffer.end();
    } catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Hellion::VulkanHelper::createTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("../Data/Textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    if(!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    vk::Buffer stagingBuffer;
    auto [stagingVertexBufferAlloc, stagingVertexBufferAllocInfo] = createBufferVma(imageSize, vk::BufferUsageFlagBits::eTransferSrc, stagingBuffer,
                                                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);

    memcpy(stagingVertexBufferAllocInfo.pMappedData, pixels, (size_t) imageSize);

    stbi_image_free(pixels);

    auto [_textureImage, _textureImageAlloc] = createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                                                           vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
    textureImage = _textureImage;
    textureImageAlloc = _textureImageAlloc;

    transitionImageLayout(vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    vmaDestroyBuffer(g_hAllocator, stagingBuffer, stagingVertexBufferAlloc);
}

std::pair<vk::Image, VmaAllocation>
Hellion::VulkanHelper::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage)
{
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = usage;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocCreateInfo.priority = 1.0f;

    vk::Image img;
    VmaAllocation alloc;
    vmaCreateImage(g_hAllocator, reinterpret_cast<VkImageCreateInfo*>(&imageInfo), &allocCreateInfo, reinterpret_cast<VkImage*>(&img), &alloc, nullptr);
    return {img, alloc};
}

void Hellion::VulkanHelper::transitionImageLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureImage;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

    endSingleTimeCommands(commandBuffer);
}

vk::CommandBuffer Hellion::VulkanHelper::beginSingleTimeCommands()
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

void Hellion::VulkanHelper::endSingleTimeCommands(vk::CommandBuffer& commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue.submit(submitInfo);

    graphicsQueue.waitIdle();

    device.freeCommandBuffers(commandPool, commandBuffer);
}

void Hellion::VulkanHelper::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
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

void Hellion::VulkanHelper::createTextureImageView()
{
    textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

vk::ImageView Hellion::VulkanHelper::createImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vk::ImageView imageView = device.createImageView(viewInfo);

    return imageView;
}

void Hellion::VulkanHelper::createTextureSampler()
{
    auto properties = physicalDevice.getProperties();

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

    textureSampler = device.createSampler(samplerInfo);
}

void Hellion::VulkanHelper::createDepthResources()
{
    vk::Format depthFormat = findDepthFormat();

    auto [_depthImage, _depthImageAlloc] = createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal,
                                                       vk::ImageUsageFlagBits::eDepthStencilAttachment);
    depthImage = _depthImage;
    depthImageAlloc = _depthImageAlloc;
    depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

vk::Format Hellion::VulkanHelper::findDepthFormat()
{
    return findSupportedFormat(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

vk::Format Hellion::VulkanHelper::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
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

