//
// Created by NePutin on 3/20/2023.
//

#include "../../include/vulkan/HPipeline.h"

void Hellion::HPipeline::createGraphicsPipeline(Hellion::PipeConf conf)
{
    auto vertShaderModule = shaderLayout[0].createShaderModule(device.getDevice());
    auto fragShaderModule = shaderLayout[1].createShaderModule(device.getDevice());

    auto vertShaderStageInfo =
            HPipelineHelper::shaderStage(vertShaderModule, vk::ShaderStageFlagBits::eVertex);
    auto fragShaderStageInfo =
            HPipelineHelper::shaderStage(fragShaderModule, vk::ShaderStageFlagBits::eFragment);
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = conf.bindingDescriptions;
    auto attributeDescriptions = conf.attributeDescriptions;
    auto vertexInputInfo = HPipelineHelper::vertexInputState(bindingDescription, attributeDescriptions);

    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &conf.inputAssemblyInfo;
    pipelineInfo.pViewportState = &conf.viewportInfo;
    pipelineInfo.pRasterizationState = &conf.rasterizationInfo;
    pipelineInfo.pMultisampleState = &conf.multisampleInfo;
    pipelineInfo.pDepthStencilState = &conf.depthStencilInfo;
    pipelineInfo.pColorBlendState = &conf.colorBlendInfo;
    pipelineInfo.pDynamicState = &conf.dynamicStateInfo;
    pipelineInfo.pDepthStencilState = &conf.depthStencilInfo;

    pipelineInfo.layout = conf.pipelineLayout;
    pipelineInfo.renderPass = conf.renderPass;
    pipelineInfo.subpass = conf.subpass;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    try
    {
        pipeline = device.getDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    device.getDevice().destroy(vertShaderModule);
    device.getDevice().destroy(fragShaderModule);
}

void Hellion::HPipeline::createGraphicsPipeline3(Hellion::PipeConf conf)
{
    auto vertShaderModule = shaderLayout[0].createShaderModule(device.getDevice());
    auto geomShaderModule = shaderLayout[1].createShaderModule(device.getDevice());
    auto fragShaderModule = shaderLayout[2].createShaderModule(device.getDevice());

    auto vertShaderStageInfo =
            HPipelineHelper::shaderStage(vertShaderModule, vk::ShaderStageFlagBits::eVertex);
    auto geomShaderStageInfo =
            HPipelineHelper::shaderStage(geomShaderModule, vk::ShaderStageFlagBits::eGeometry);
    auto fragShaderStageInfo =
            HPipelineHelper::shaderStage(fragShaderModule, vk::ShaderStageFlagBits::eFragment);
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo, geomShaderStageInfo};

    auto bindingDescription = conf.bindingDescriptions;
    auto attributeDescriptions = conf.attributeDescriptions;
    auto vertexInputInfo = HPipelineHelper::vertexInputState(bindingDescription, attributeDescriptions);

    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.stageCount = 3;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &conf.inputAssemblyInfo;
    pipelineInfo.pViewportState = &conf.viewportInfo;
    pipelineInfo.pRasterizationState = &conf.rasterizationInfo;
    pipelineInfo.pMultisampleState = &conf.multisampleInfo;
    pipelineInfo.pDepthStencilState = &conf.depthStencilInfo;
    pipelineInfo.pColorBlendState = &conf.colorBlendInfo;
    pipelineInfo.pDynamicState = &conf.dynamicStateInfo;
    pipelineInfo.pDepthStencilState = &conf.depthStencilInfo;

    pipelineInfo.layout = conf.pipelineLayout;
    pipelineInfo.renderPass = conf.renderPass;
    pipelineInfo.subpass = conf.subpass;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    try
    {
        pipeline = device.getDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;
    }
    catch (vk::SystemError err)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    device.getDevice().destroy(vertShaderModule);
    device.getDevice().destroy(geomShaderModule);
    device.getDevice().destroy(fragShaderModule);
}
