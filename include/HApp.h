//
// Created by NePutin on 3/24/2023.
//

#ifndef HELLION_HAPP_H
#define HELLION_HAPP_H

#include "vulkan/HPipeline.h"
#include "vulkan/HDescriptorSetLayout.h"

namespace Hellion
{

    class HApp
    {
    public:
        HApp()
        {
            createGraphicsPipeline();
//            createCommandBuffers();
        }

        ~HApp()
        {

        }

        void run()
        {}

        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

    private:
        void createGraphicsPipeline()
        {
            auto globalPool =
                    HDescriptorPool::Builder(device)
                            .setMaxSets(HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .addPoolSize(vk::DescriptorType::eUniformBuffer, HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .addPoolSize(vk::DescriptorType::eCombinedImageSampler, HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .build();

            auto globalSetLayout =
                    HDescriptorSetLayout::Builder(device)
                            .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                            .addBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                            .build();


            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalSetLayout->getDescriptorSetLayout()};

            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;

            pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
            auto configInfo = PipeConf::createDefault2(swapChain);
            configInfo.pipelineLayout = pipelineLayout;
            configInfo.renderPass = swapChain.getRenderPass();

            pipeline =
                    std::make_unique<HPipeline>(device, std::array<HShader, 2>{HShader("../Data/Shaders/vert.spv"), HShader("../Data/Shaders/frag.spv")},
                                                std::move(configInfo));
        }

        void createCommandBuffers()
        {

        }

//        void drawFrame()
//        {
//            uint32_t imageIndex = swapChain_.acquireNextImage();
//            swapChain_.submitCommandBuffers(commandBuffers_[imageIndex], imageIndex);
//        }

        HWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
        HDevice device{window};
        HSwapChain swapChain{window, device};

        vk::PipelineLayout pipelineLayout;
        std::unique_ptr<HPipeline> pipeline;
    };

} // Hellion

#endif //HELLION_HAPP_H
