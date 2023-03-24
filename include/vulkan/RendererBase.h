//
// Created by NePutin on 3/19/2023.
//

#ifndef HELLION_RENDERERBASE_H
#define HELLION_RENDERERBASE_H

#include "HDevice.h"
#include "HImage.h"

namespace Hellion
{
    constexpr inline int MAX_FRAMES_IN_FLIGHT = 2;
    class RendererBase
    {
    public:
        explicit RendererBase(HDevice& vkDev, HellImage depthTexture)
                : device_(vkDev.getDevice()), framebufferWidth_(vkDev.framebufferWidth), framebufferHeight_(vkDev.framebufferHeight),
                  depthTexture_(depthTexture)
        {}

        virtual ~RendererBase()
        {

        }

        virtual void fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) = 0;

        inline HellImage getDepthTexture() const
        { return depthTexture_; }

    protected:
        void beginRenderPass(vk::CommandBuffer& commandBuffer, size_t currentImage)
        {
            const VkRect2D screenRect = {
                    .offset = {0, 0},
                    .extent = {.width = framebufferWidth_, .height = framebufferHeight_}
            };

            vk::RenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.renderPass = renderPass_;
            renderPassInfo.framebuffer = swapchainFramebuffers_[currentImage];
            renderPassInfo.renderArea = screenRect;

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline_);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout_, 0, 1, &descriptorSets_[currentImage], 0, nullptr);
        }

        bool createUniformBuffers(HDevice& vkDev, size_t uniformDataSize)
        {}

        vk::Device device_ = nullptr;

        uint32_t framebufferWidth_ = 0;
        uint32_t framebufferHeight_ = 0;

        // Depth buffer
        HellImage depthTexture_;

        // Descriptor set (layout + pool + sets) -> uses uniform buffers, textures, framebuffers
        vk::DescriptorSetLayout descriptorSetLayout_{nullptr};
        vk::DescriptorPool descriptorPool_{nullptr};
        std::vector<vk::DescriptorSet> descriptorSets_;

        // Framebuffers (one for each command buffer)
        std::vector<vk::Framebuffer> swapchainFramebuffers_;

        // 4. Pipeline & render pass (using DescriptorSets & pipeline state options)
        vk::RenderPass renderPass_{nullptr};
        vk::PipelineLayout pipelineLayout_{nullptr};
        vk::Pipeline graphicsPipeline_{nullptr};

        // 5. Uniform buffer
        std::vector<vk::Buffer> uniformBuffers_;
        std::vector<VmaAllocation> uniformBuffersAllocations_;
    };
}

#endif //HELLION_RENDERERBASE_H
