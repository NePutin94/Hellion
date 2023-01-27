//
// Created by NePutin on 1/28/2023.
//

#ifndef HELLION_VULKANHELPER_H
#define HELLION_VULKANHELPER_H

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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

        const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;
#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif
    private:
        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

        void pickPhysicalDevice();

        void createLogicalDevice();

        bool isDeviceSuitable(VkPhysicalDevice device);

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        std::vector<const char*> getRequiredExtensions();

        bool checkValidationLayerSupport();

        void setupDebugMessenger();

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        void createInstance();

        void createSurface(GLFWwindow* window);
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

        static VkResult
        CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                     VkDebugUtilsMessengerEXT* pDebugMessenger)
        {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if(func != nullptr)
            {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            } else
            {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }


    };
}
#endif //HELLION_VULKANHELPER_H
