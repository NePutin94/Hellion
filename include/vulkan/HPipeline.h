//
// Created by NePutin on 3/20/2023.
//

#ifndef HELLION_HPIPELINE_H
#define HELLION_HPIPELINE_H

#include <vulkan/vulkan.hpp>
#include "HShader.h"
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
        std::vector<HShader> shaderLayout;
    public:
        HPipeline(HDevice& device, std::array<HShader, 2> shaderLayout, PipeConf configInfo) : device{device}
        {
            std::move(shaderLayout.begin(), shaderLayout.end(), std::back_inserter(this->shaderLayout));
            createGraphicsPipeline(std::move(configInfo));
        }

        HPipeline(HDevice& device, std::array<HShader, 3> shaderLayout, PipeConf configInfo) : device{device}
        {
            std::move(shaderLayout.begin(), shaderLayout.end(), std::back_inserter(this->shaderLayout));
            createGraphicsPipeline3(std::move(configInfo));
        }

        ~HPipeline()
        { device.getDevice().destroy(pipeline); }

        void bind(vk::CommandBuffer commandBuffer)
        { commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline); }

        void createGraphicsPipeline(PipeConf conf);

        void createGraphicsPipeline3(PipeConf conf);
    };
}

#endif //HELLION_HPIPELINE_H
