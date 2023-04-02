//
// Created by NePutin on 4/2/2023.
//

#ifndef HELLION_CANVASSYSTEM_H
#define HELLION_CANVASSYSTEM_H

#include <glm/common.hpp>
#include "HDevice.h"
#include "HPipeline.h"
#include "HDescriptorSetLayout.h"
#include "HBuffer.h"
#include "HTexture.h"
#include "../core/Profiling.h"

namespace Hellion
{
    class CanvasSystem
    {
    public:
        CanvasSystem(HDevice& device)
                : device{device}
        {}

        ~CanvasSystem()
        {
            device.getDevice().destroy(pipelineLayout);
        }

        void init(vk::RenderPass renderPass, HSwapChain& swapchain)
        {
            createPipelineLayout();
            createPipeline(renderPass, swapchain);
            createVertexBufferVma();
        }

        void clear();

        void line(glm::vec3 p1, glm::vec3 p2, glm::vec4 c);

        void plane3d(const glm::vec3& orig, const glm::vec3& v1, const glm::vec3& v2, int n1, int n2, float s1, float s2, const glm::vec4& color,
                     const glm::vec4& outlineColor);

        void createPipeline(vk::RenderPass renderPass, HSwapChain& swapchain)
        {
            PipeConf pipelineConfig = PipeConf::createDefaultLine(swapchain);
            pipelineConfig.renderPass = renderPass;
            pipelineConfig.pipelineLayout = pipelineLayout;
            pipeline = std::make_unique<HPipeline>(device, std::array<HShader, 2>{HShader("../Data/Shaders/LineV.spv"), HShader("../Data/Shaders/LineF.spv")},
                                                   std::move(pipelineConfig));
        }

        void createPipelineLayout()
        {
            HELLION_ZONE_PROFILING()

            globalPool =
                    HDescriptorPool::Builder(device)
                            .setMaxSets(HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .addPoolSize(vk::DescriptorType::eUniformBuffer, HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .build();

            uboBuffers.resize(HSwapChain::MAX_FRAMES_IN_FLIGHT);

            for(int i = 0; i < uboBuffers.size(); i++)
            {
                uboBuffers[i] = std::make_unique<HBuffer>(
                        device,
                        sizeof(UniformBuffer),
                        vk::BufferUsageFlagBits::eUniformBuffer,
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                        VMA_ALLOCATION_CREATE_MAPPED_BIT);
            }

            renderSystemLayout =
                    HDescriptorSetLayout::Builder(device)
                            .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                            .build();

            globalDescriptorSets.resize(HSwapChain::MAX_FRAMES_IN_FLIGHT);
            for(int i = 0; i < globalDescriptorSets.size(); i++)
            {
                auto bufferInfo = uboBuffers[i]->descriptorInfo();
                HDescriptorWriter(*renderSystemLayout, *globalPool)
                        .writeBuffer(0, &bufferInfo)
                        .build(globalDescriptorSets[i]);
            }

            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &renderSystemLayout->getDescriptorSetLayout();
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;

            pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
        }

        void updateBuffers(uint32_t currentFrame, float width, float height)
        {
            HELLION_ZONE_PROFILING()
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            UniformBuffer ubo{};
            ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
            ubo.proj[1][1] *= -1;
            ubo.time = time * 15;
            memcpy(uboBuffers[currentFrame]->getMappedMemory(), &ubo, sizeof(ubo));
        }

        void draw(vk::CommandBuffer& buffer, uint32_t currentFrame, tracy::VkCtx* tracyCtx);

        void createVertexBufferVma()
        {
            HELLION_ZONE_PROFILING()
            vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
            auto stagingBuffer = HBuffer(device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);

            memcpy(stagingBuffer.getMappedMemory(), vertices.data(), (size_t) bufferSize);
            vertexBuffer = std::make_unique<HBuffer>(device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, 0);

            device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
        }

    private:
        HDevice& device;
        std::unique_ptr<HPipeline> pipeline;
        vk::PipelineLayout pipelineLayout;
        std::unique_ptr<HDescriptorPool> globalPool;
        std::vector<std::unique_ptr<HBuffer>> uboBuffers;
        std::vector<HVertexLine> vertices;
        std::unique_ptr<HBuffer> vertexBuffer;
        std::vector<vk::DescriptorSet> globalDescriptorSets;
        std::unique_ptr<HDescriptorSetLayout> renderSystemLayout;

        struct UniformBuffer
        {
            alignas(16) glm::mat4 model;
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 proj;
            alignas(16) float time;
        };
    };
} // Hellion

#endif //HELLION_CANVASSYSTEM_H
