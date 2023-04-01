//
// Created by NePutin on 3/24/2023.
//

#ifndef HELLION_HPIPELINEHELPER_H
#define HELLION_HPIPELINEHELPER_H

#include <vulkan/vulkan.hpp>
#include "HSwapChain.h"

namespace Hellion
{
    struct HPipelineHelper
    {
        static vk::PipelineVertexInputStateCreateInfo
        vertexInputState(vk::VertexInputBindingDescription& bindingDescription, std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions)
        {
            vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
            vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
            return vertexInputInfo;
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

        static vk::PipelineViewportStateCreateInfo viewportState()
        {
            vk::PipelineViewportStateCreateInfo viewportState = {};
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;
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

        static vk::PipelineShaderStageCreateInfo shaderStage(vk::ShaderModule

        module,
        vk::ShaderStageFlagBits stage
        )
        {
            vk::PipelineShaderStageCreateInfo shaderStage = {};
            shaderStage.stage = stage;
            shaderStage.module = module;
            shaderStage.pName = "main";
            return shaderStage;
        }
    };

    struct PipeConf
    {
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
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
        std::vector<vk::DynamicState> dynamicStateEnables;
        vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
        vk::PipelineViewportStateCreateInfo viewportInfo;

        PipeConf() = default;

        PipeConf(const PipeConf&) = delete;

        PipeConf(PipeConf&&) = default;

        PipeConf& operator=(const PipeConf&) = delete;

        PipeConf& operator=(PipeConf&&) = default;

        static PipeConf createDefault(HSwapChain& swapChain)
        {
            PipeConf configInfo{};
            configInfo.inputAssemblyInfo = HPipelineHelper::inputAssemblyState();
            auto swapChainExtent = swapChain.getSwapChainExtent();
            configInfo.viewport = vk::Viewport{0.0f, 0.0f, (float) swapChainExtent.width, (float) swapChainExtent.height, 0.0f, 1.0f};
            configInfo.scissor = vk::Rect2D{{0, 0}, swapChainExtent};
            configInfo.rasterizationInfo = HPipelineHelper::rasterizationState();
            configInfo.multisampleInfo = HPipelineHelper::multisampleState();
            configInfo.colorBlendAttachment = HPipelineHelper::colorBlendAttachmentState();
            configInfo.colorBlendInfo = HPipelineHelper::colorBlendingState(configInfo.colorBlendAttachment);
            configInfo.depthStencilInfo = HPipelineHelper::depthStencilState();
            configInfo.subpass = 0;
            configInfo.renderPass = swapChain.getRenderPass();
            configInfo.dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
            configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
            configInfo.dynamicStateInfo.dynamicStateCount =
                    static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
            configInfo.viewportInfo = HPipelineHelper::viewportState();
            return configInfo;
        }

        static PipeConf createDefault2(HSwapChain& swapChain)
        {
            PipeConf configInfo{};

            configInfo.inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
            configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

            configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
            configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
            configInfo.rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
            configInfo.rasterizationInfo.lineWidth = 1.0f;
            configInfo.rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
            configInfo.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
            configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;

            configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
            configInfo.multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

            configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
            configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
            configInfo.depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
            configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
            configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;

            configInfo.colorBlendAttachment.colorWriteMask =
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
            configInfo.colorBlendAttachment.blendEnable = VK_FALSE;

            configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
            configInfo.colorBlendInfo.logicOp = vk::LogicOp::eCopy;
            configInfo.colorBlendInfo.attachmentCount = 1;
            configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
            configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
            configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
            configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
            configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

            configInfo.dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
            configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
            configInfo.dynamicStateInfo.dynamicStateCount =
                    static_cast<uint32_t>(configInfo.dynamicStateEnables.size());

            configInfo.viewportInfo.viewportCount = 1;
            configInfo.viewportInfo.scissorCount = 1;

            auto swapChainExtent = swapChain.getSwapChainExtent();
            configInfo.subpass = 0;
            configInfo.renderPass = swapChain.getRenderPass();

            return configInfo;
        }
    };
}

#endif //HELLION_HPIPELINEHELPER_H
