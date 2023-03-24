//
// Created by NePutin on 3/20/2023.
//

#ifndef HELLION_HPIPELINE_H
#define HELLION_HPIPELINE_H

#include <vulkan/vulkan.hpp>
#include "HShader.h"
#include "VulkanHelper.h"
#include "HSwapChain.h"

namespace Hellion
{
    class HPipeline
    {
    private:
        vk::Pipeline pipeline;
    public:

        struct PipeConf
        {
            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
            vk::Viewport viewport;
            vk::Rect2D scissor;
            vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
            vk::PipelineMultisampleStateCreateInfo multisampleInfo;
            vk::PipelineColorBlendAttachmentState colorBlendAttachment;
            vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
            vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
            vk::RenderPass renderPass;
            vk::PipelineLayout pipelineLayout;
            uint32_t subpass = 0;

            static PipeConf createDefault(HSwapChain& swapChain)
            {
                PipeConf configInfo{};
                configInfo.inputAssemblyInfo = HPipeline::inputAssemblyState();
                auto swapChainExtent = swapChain.getSwapChainExtent();
                configInfo.viewport = vk::Viewport{0.0f, 0.0f, (float) swapChainExtent.width, (float) swapChainExtent.height, 0.0f, 1.0f};
                configInfo.scissor = vk::Rect2D{{0, 0}, swapChainExtent};
                configInfo.rasterizationInfo = HPipeline::rasterizationState();
                configInfo.multisampleInfo = HPipeline::multisampleState();
                configInfo.colorBlendAttachment = HPipeline::colorBlendAttachmentState();
                configInfo.colorBlendInfo = HPipeline::colorBlendingState(configInfo.colorBlendAttachment);
                configInfo.depthStencilInfo = HPipeline::depthStencilState();
                configInfo.subpass = 0;
                configInfo.renderPass = swapChain.getRenderPass();
            }
        };

        HPipeline(HDevice& device, std::array<HShader, 2> shaderLayout, PipeConf& configInfo)
        {
            createGraphicsPipeline(configInfo, std::move(shaderLayout));
        }

        static vk::PipelineVertexInputStateCreateInfo
        vertexInputState(vk::VertexInputBindingDescription& bindingDescription, std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions)
        {
            vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
            vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        }

        static vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState()
        {
            vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
            inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
            inputAssembly.primitiveRestartEnable = VK_FALSE;
            return inputAssembly;
        }

        static vk::PipelineViewportStateCreateInfo viewportState(vk::Viewport& viewport, vk::Rect2D& scissor)
        {
            vk::PipelineViewportStateCreateInfo viewportState = {};
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.pScissors = &scissor;
            return viewportState;
        }

        static vk::PipelineRasterizationStateCreateInfo rasterizationState()
        {
            vk::PipelineRasterizationStateCreateInfo rasterizer = {};
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = vk::PolygonMode::eFill;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = vk::CullModeFlagBits::eBack;
            rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
            rasterizer.depthBiasEnable = VK_FALSE;
            return rasterizer;
        }

        static vk::PipelineMultisampleStateCreateInfo multisampleState()
        {
            vk::PipelineMultisampleStateCreateInfo multisampling = {};
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
            return multisampling;
        }

        static vk::PipelineColorBlendAttachmentState colorBlendAttachmentState()
        {
            vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.colorWriteMask =
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
            colorBlendAttachment.blendEnable = VK_FALSE;
            return colorBlendAttachment;
        }

        static vk::PipelineColorBlendStateCreateInfo colorBlendingState(
                vk::PipelineColorBlendAttachmentState& colorBlendAttachment)
        {
            vk::PipelineColorBlendStateCreateInfo colorBlending = {};
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = vk::LogicOp::eCopy;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;
            return colorBlending;
        }

        static vk::PipelineDepthStencilStateCreateInfo depthStencilState()
        {
            vk::PipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = vk::CompareOp::eLess;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;
            return depthStencil;
        }

        static vk::PipelineLayoutCreateInfo pipelineLayoutInfo()
        {
            vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pSetLayouts = nullptr;
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;
            return pipelineLayoutInfo;
        }

        static vk::PipelineLayoutCreateInfo
        pipelineLayoutInfo(std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, std::vector<vk::PushConstantRange>& pushConstants)
        {
            vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
            pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
            pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
            return pipelineLayoutInfo;
        }

        static vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
                const vk::DescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount = 1)
        {
            vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
            pipelineLayoutInfo.setLayoutCount = setLayoutCount;
            pipelineLayoutInfo.pSetLayouts = pSetLayouts;
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;
            return pipelineLayoutInfo;
        }

        void createGraphicsPipeline(PipeConf conf, std::array<HShader, 2> shaders)
        {
            vk::PipelineShaderStageCreateInfo shaderStages[] = {
                    {
                            vk::PipelineShaderStageCreateFlags(),
                            vk::ShaderStageFlagBits::eVertex,
                            *shaders[0].getShaderModule(),
                            "main"
                    },
                    {
                            vk::PipelineShaderStageCreateFlags(),
                            vk::ShaderStageFlagBits::eFragment,
                            *shaders[0].getShaderModule(),
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

        }

    };
}

#endif //HELLION_HPIPELINE_H
