//
// Created by NePutin on 1/28/2023.
//

#ifndef HELLION_VULKANHELPER_H
#define HELLION_VULKANHELPER_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <optional>
#include <fmt/core.h>
#include <vector>
#include <set>
#include <limits>
#include <fstream>

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

        struct SwapChainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> presentModes;
        };

        vk::PhysicalDevice physicalDevice{nullptr};
        vk::Instance instance{nullptr};
        vk::DebugUtilsMessengerEXT debugMessenger{nullptr};
        vk::DispatchLoaderDynamic dldi;
        vk::SurfaceKHR surface;
        vk::Device device;

        vk::Queue presentQueue;
        vk::Queue graphicsQueue;

        vk::SwapchainKHR swapChain;
        std::vector<vk::Image> swapChainImages;
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;

        std::vector<vk::ImageView> swapChainImageViews;
        std::vector<vk::Framebuffer> swapChainFramebuffers;

        vk::RenderPass renderPass;
        vk::PipelineLayout pipelineLayout;
        vk::Pipeline graphicsPipeline;

        VkCommandPool commandPool;
        std::vector<vk::CommandBuffer, std::allocator<vk::CommandBuffer>> commandBuffers;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;
        size_t currentFrame = 0;

        const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };
        const int MAX_FRAMES_IN_FLIGHT = 2;
        const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

        bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device);

        void pickPhysicalDevice();

        bool isDeviceSuitable(const vk::PhysicalDevice& device);

        QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device);

        void createLogicalDevice();

        void createSurface(GLFWwindow* window);

        void createSwapChain(GLFWwindow* window);

        SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device);

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& vector1);

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& vector1);

        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& khr, GLFWwindow* window);

        void createImageViews();

        void createGraphicsPipeline();

    public:
        VulkanHelper() = default;

        void cleanup();

        void init(GLFWwindow* window);

        void drawFrame();

        void wait()
        {
            device.waitIdle();
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
        {
            fmt::println("validation layer: {}", pCallbackData->pMessage);
            return VK_FALSE;
        }

        static std::vector<char> readFile(const std::string& filename)
        {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if(!file.is_open())
            {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            file.close();

            return buffer;
        }

        vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);

        void createRenderPass();

        void createFramebuffers();

        void createCommandPool();

        void createCommandBuffers();

        void createSyncObjects();
    };
}
#endif //HELLION_VULKANHELPER_H
