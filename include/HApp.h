//
// Created by NePutin on 3/24/2023.
//

#ifndef HELLION_HAPP_H
#define HELLION_HAPP_H

#include "vulkan/HPipeline.h"
#include "vulkan/HDescriptorSetLayout.h"
#include "vulkan/HBuffer.h"
#include "vulkan/HRenderer.h"

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
            std::vector<std::unique_ptr<HBuffer>> uboBuffers(HSwapChain::MAX_FRAMES_IN_FLIGHT);

            for (int i = 0; i < uboBuffers.size(); i++) {
                uboBuffers[i] = std::make_unique<HBuffer>(
                        device,
                        sizeof(UniformBufferObject),
                        vk::BufferUsageFlagBits::eUniformBuffer,
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                        VMA_ALLOCATION_CREATE_MAPPED_BIT);
                uboBuffers[i]->map();
            }

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


            std::vector<vk::DescriptorSet> globalDescriptorSets(HSwapChain::MAX_FRAMES_IN_FLIGHT);
            for (int i = 0; i < globalDescriptorSets.size(); i++) {
                auto bufferInfo = uboBuffers[i]->descriptorInfo();
                HDescriptorWriter(*globalSetLayout, *globalPool)
                        .writeBuffer(0, &bufferInfo)
                        .build(globalDescriptorSets[i]);
            }

            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalSetLayout->getDescriptorSetLayout()};

            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;

            pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
            auto configInfo = PipeConf::createDefault2(*renderer.getSwapChain());
            configInfo.pipelineLayout = pipelineLayout;
            configInfo.renderPass = renderer.getSwapChainRenderPass();

            pipeline =
                    std::make_unique<HPipeline>(device, std::array<HShader, 2>{HShader("../Data/Shaders/vert.spv"), HShader("../Data/Shaders/frag.spv")},
                                                std::move(configInfo));
        }

        void drawFrame()
        {
            while(true)
            {
                if(auto commandBuffer = renderer.beginFrame())
                {
                    int frameIndex = renderer.getFrameIndex();
                    renderer.beginSwapChainRenderPass(commandBuffer);


                    renderer.endSwapChainRenderPass(commandBuffer);
                    renderer.endFrame();
                }
            }
        }

        HWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
        HDevice device{window};
        HRenderer renderer{window, device};
        //HSwapChain swapChain{window, device};

        vk::PipelineLayout pipelineLayout;
        std::unique_ptr<HPipeline> pipeline;
        struct UniformBufferObject
        {
            alignas(16) glm::mat4 model;
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 proj;
            alignas(16) float time;
        };

        std::vector<HBuffer> uniformBuffers;
    };

} // Hellion

#endif //HELLION_HAPP_H
