//
// Created by NePutin on 3/20/2023.
//

#ifndef HELLION_HPIPELINE_H
#define HELLION_HPIPELINE_H

#include <vulkan/vulkan.hpp>
#include "HShader.h"
#include "VulkanHelper.h"
#include "HSwapChain.h"
#include "HPipelineHelper.h"
#include "HVertex.h"
#include <vector>

namespace Hellion
{
    class HPipeline
    {
    private:
        vk::Pipeline pipeline;
        HDevice& device;
        std::array<HShader, 2> shaderLayout;
    public:
        HPipeline(HDevice& device, std::array<HShader, 2> shaderLayout, PipeConf& configInfo) : device{device}, shaderLayout{shaderLayout}
        {
            createGraphicsPipeline(configInfo, std::move(shaderLayout));
        }

        ~HPipeline()
        { device.getDevice().destroy(pipeline); }

        void bind(vk::CommandBuffer commandBuffer)
        { commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline); }

        void createGraphicsPipeline(PipeConf conf, std::array<HShader, 2> shaders);
    };
}

#endif //HELLION_HPIPELINE_H
