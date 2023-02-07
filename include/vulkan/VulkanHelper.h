//
// Created by NePutin on 1/28/2023.
//

#ifndef HELLION_VULKANHELPER_H
#define HELLION_VULKANHELPER_H

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <optional>
#include <fmt/core.h>
#include <vector>
#include <set>
#include <limits>
#include <fstream>
#include <glm/glm.hpp>
#include <vk_mem_alloc.h>

namespace Hellion
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static vk::VertexInputBindingDescription getBindingDescription()
        {
            vk::VertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = vk::VertexInputRate::eVertex;

            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }
    };

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

        struct UniformBufferObject
        {
            alignas(16) glm::mat4 model;
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 proj;
            alignas(16) float time;
        };

        std::vector<vk::Buffer> uniformBuffers;
        std::vector<void*> uniformBuffersMapped;
        vk::DescriptorSetLayout descriptorSetLayout;
        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;
        std::vector<VmaAllocation> uniformBuffersAllocs;

        vk::Image depthImage;
        vk::ImageView depthImageView;
        VmaAllocation depthImageAlloc;

        vk::Image textureImage;
        VmaAllocation textureImageAlloc;
        vk::ImageView textureImageView;
        vk::Sampler textureSampler;

        VmaAllocator g_hAllocator;

        vk::Buffer vertexBuffer;
        VmaAllocation vertexAllocation;
        vk::Buffer indexBuffer;
        VmaAllocation indexAllocation;

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

        //geometry
        const std::vector<Vertex> vertices = {
                {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

                {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };

        const std::vector<uint16_t> indices = {
                0, 1, 2, 2, 3, 0,
                4, 5, 6, 6, 7, 4
        };

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

        bool framebufferResized = false;
    public:
        VulkanHelper() = default;

        void cleanup();

        void init(GLFWwindow* window);

        void drawFrame(GLFWwindow* window);

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

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
        {
            auto app = reinterpret_cast<VulkanHelper*>(glfwGetWindowUserPointer(window));
            app->framebufferResized = true;
        }

        vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);

        void createRenderPass();

        void createFramebuffers();

        void createCommandPool();

        void createCommandBuffers();

        void createSyncObjects();

        void recreateSwapChain(GLFWwindow* window);

        void cleanupSwapChain();

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

        void createVmaAllocator();

        std::pair<VmaAllocation, VmaAllocationInfo>
        createBufferVma(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::Buffer& buffer, VmaAllocationCreateFlags flags);

        void createVertexBufferVma();

        void createIndexBufferVma();

        void createUniformBuffers();

        void createDescriptorSetLayout();

        void updateUniformBuffer(uint32_t currentImage);

        void createDescriptorPool();

        void createDescriptorSets();

        void recordCommandBuffer(vk::CommandBuffer& buffer, uint32_t imageIndex);

        void createTextureImage();

        std::pair<vk::Image, VmaAllocation> createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage);

        void transitionImageLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

        vk::CommandBuffer beginSingleTimeCommands();

        void endSingleTimeCommands(vk::CommandBuffer& buffer);

        void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

        void createTextureImageView();

        vk::ImageView createImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags);

        void createTextureSampler();

        void createDepthResources();

        vk::Format findDepthFormat();

        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    };
}
#endif //HELLION_VULKANHELPER_H
