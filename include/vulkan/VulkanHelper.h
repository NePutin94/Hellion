//
// Created by NePutin on 1/28/2023.
//

#ifndef HELLION_VULKANHELPER_H
#define HELLION_VULKANHELPER_H

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#ifdef _WIN64
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.hpp>
#include <optional>
#include <fmt/core.h>
#include <vector>

namespace Hellion
{
    class VulkanHelper
    {
    private:
        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool isComplete()
            {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        vk::UniqueInstance instance;
        VkDebugUtilsMessengerEXT callback;
        vk::SurfaceKHR surface;

        vk::PhysicalDevice physicalDevice;
        vk::UniqueDevice device;

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;


        const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };

#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif
    private:

        std::vector<const char*> getRequiredExtensions();

        bool supported(std::vector<const char*>& extensions, const std::vector<const char*>& layers, bool debug);

        void setupDebug();

        void createInstance();

        bool isSuitable(const vk::PhysicalDevice& device);

        bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& requestedExtensions);

    public:
        VulkanHelper() = default;

        void cleanup();

        void init(GLFWwindow* window);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
        {
            fmt::println("validation layer: {}", pCallbackData->pMessage);
            return VK_FALSE;
        }

        VkResult
        CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                     VkDebugUtilsMessengerEXT* pCallback)
        {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if(func != nullptr)
            {
                return func(instance, pCreateInfo, pAllocator, pCallback);
            } else
            {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }

        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if(func != nullptr)
            {
                func(instance, callback, pAllocator);
            }
        }

        void pickPhysicalDevice();

        bool isDeviceSuitable(const vk::PhysicalDevice& device);

        QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device);

        void createLogicalDevice();

        void createSurface(GLFWwindow* window);

        bool checkValidationLayerSupport();
    };
}
#endif //HELLION_VULKANHELPER_H
