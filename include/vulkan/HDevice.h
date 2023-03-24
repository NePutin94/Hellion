//
// Created by NePutin on 2/19/2023.
//

#ifndef HELLION_HDEVICE_H
#define HELLION_HDEVICE_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <fmt/core.h>
#include "HWindow.h"
#include <optional>
#include <set>
#include <vk_mem_alloc.h>

namespace Hellion
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    class HDevice
    {
    private:
        const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };
        const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VmaAllocator g_hAllocator;

        vk::PhysicalDevice physicalDevice{nullptr};
        vk::Device device;

        vk::Queue presentQueue;
        vk::Queue graphicsQueue;

        vk::CommandPool commandPool;

        vk::Instance instance{nullptr};
        vk::DebugUtilsMessengerEXT debugMessenger{nullptr};
        vk::DispatchLoaderDynamic dldi;
        bool enableValidationLayers = true;

        HWindow window;
        vk::SurfaceKHR surface;

    public:
        uint32_t framebufferWidth;
        uint32_t framebufferHeight;

        HDevice(HWindow& windowRef) : window{windowRef}
        { init(); }

        void cleanup();

        ~HDevice()
        { cleanup(); }

        vk::PipelineLayout createPipelineLayout(vk::PipelineLayoutCreateInfo info)
        { return device.createPipelineLayout(info); }

        vk::Device getDevice()
        { return device; }

        vk::Queue getPresentQueue()
        { return presentQueue; }

        vk::Queue getGraphicsQueue()
        { return graphicsQueue; }

        vk::SurfaceKHR& getSurface()
        { return surface; }

        vk::CommandPool getCommandPool()
        { return commandPool; }

        VmaAllocator getAllocator()
        { return g_hAllocator; }

        bool hasStencilComponent(vk::Format format)
        { return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint; }

        SwapChainSupportDetails getSwapChainSupport()
        { return querySwapChainSupport(physicalDevice); }

        QueueFamilyIndices findPhysicalQueueFamilies()
        { return findQueueFamilies(physicalDevice); }

        void createSurface()
        { window.createWindowSurface(instance, &surface); }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
        {
            fmt::println("validation layer: {}", pCallbackData->pMessage);
            return VK_FALSE;
        }

        static constexpr uint32_t GetVulkanApiVersion()
        {
#if VMA_VULKAN_VERSION == 1003000
            return VK_API_VERSION_1_3;
#elif VMA_VULKAN_VERSION == 1002000
            return VK_API_VERSION_1_2;
#elif VMA_VULKAN_VERSION == 1001000
    return VK_API_VERSION_1_1;
#elif VMA_VULKAN_VERSION == 1000000
    return VK_API_VERSION_1_0;
#else
#error Invalid VMA_VULKAN_VERSION.
    return UINT32_MAX;
#endif
        }

        std::pair<VmaAllocation, VmaAllocationInfo>
        createBufferVma(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::Buffer& buffer, VmaAllocationCreateFlags flags);

        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

        vk::CommandBuffer beginSingleTimeCommands();

        void endSingleTimeCommands(vk::CommandBuffer& commandBuffer);

        std::pair<vk::Image, VmaAllocation>
        createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                    vk::ImageCreateFlags flags, uint32_t arrayLayers);

        void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels,
                                   uint32_t layerCount);

        void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

        vk::ImageView createImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels, vk::ImageViewType viewType);

        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

    private:
        void init();

        void createInstance();

        void setupDebug();

        void pickPhysicalDevice();

        void createLogicalDevice();

        void createVmaAllocator();

        void createCommandPool();

        bool supported(std::vector<const char*>& extensions, const std::vector<const char*>& layers, bool debug);

        std::vector<const char*> getRequiredExtensions();

        QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device);

        bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device);

        SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device);

        bool isDeviceSuitable(const vk::PhysicalDevice& device);
    };
}

#endif //HELLION_HDEVICE_H
