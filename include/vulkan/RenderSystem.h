//
// Created by NePutin on 3/28/2023.
//

#ifndef HELLION_RENDERSYSTEM_H
#define HELLION_RENDERSYSTEM_H

#include "HDevice.h"
#include "HPipeline.h"
#include "HDescriptorSetLayout.h"
#include "HBuffer.h"
#include "tiny_obj_loader.h"
#include "HTexture.h"

namespace Hellion
{

    class RenderSystem
    {
    public:
        RenderSystem(HDevice& device, vk::RenderPass renderPass, HSwapChain& swapchain)
                : device{device}
        {
            createPipelineLayout();
            createPipeline(renderPass, swapchain);
            loadModel();
            createVertexBufferVma();
        }

        ~RenderSystem()
        {
            device.getDevice().destroy(pipelineLayout);
        }

        RenderSystem(const RenderSystem&) = delete;

        RenderSystem& operator=(const RenderSystem&) = delete;

        void renderGameObjects(vk::CommandBuffer& buffer)
        {
//            pipeline->bind(buffer);
//
//            buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
//
//            vk::Buffer vertexBuffers[] = {vertexBuffer};
//
//            vk::DeviceSize offsets[] = {0};
//
//            buffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
//
//            buffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
//
//            buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
//
//            buffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        }

    private:
        struct UniformBufferObject
        {
            alignas(16) glm::mat4 model;
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 proj;
            alignas(16) float time;
        };

        void createVertexBufferVma()
        {
            vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
            auto stagingBuffer = HBuffer(device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);

            memcpy(stagingBuffer.getMappedMemory(), vertices.data(), (size_t) bufferSize);
            vertexBuffer = std::make_unique<HBuffer>(device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, 0);

            device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
        }

        void createIndexBufferVma()
        {

        }

        void createPipelineLayout()
        {
            texture = HTexture::createTextureFromFile(device, TEXTURE_PATH.c_str());

            globalPool =
                    HDescriptorPool::Builder(device)
                            .setMaxSets(HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .addPoolSize(vk::DescriptorType::eUniformBuffer, HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .addPoolSize(vk::DescriptorType::eCombinedImageSampler, HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .build();
            std::vector<std::unique_ptr<HBuffer>> uboBuffers(HSwapChain::MAX_FRAMES_IN_FLIGHT);

            for(int i = 0; i < uboBuffers.size(); i++)
            {
                uboBuffers[i] = std::make_unique<HBuffer>(
                        device,
                        sizeof(UniformBufferObject),
                        vk::BufferUsageFlagBits::eUniformBuffer,
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                        VMA_ALLOCATION_CREATE_MAPPED_BIT);
            }

            renderSystemLayout =
                    HDescriptorSetLayout::Builder(device)
                            .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                            .addBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
                            .build();

            std::vector<vk::DescriptorSet> globalDescriptorSets(HSwapChain::MAX_FRAMES_IN_FLIGHT);
            for(int i = 0; i < globalDescriptorSets.size(); i++)
            {
                auto imageInfo = texture->getImageInfo();
                auto bufferInfo = uboBuffers[i]->descriptorInfo();
                HDescriptorWriter(*renderSystemLayout, *globalPool)
                        .writeBuffer(0, &bufferInfo)
                        .writeImage(1, &imageInfo)
                        .build(globalDescriptorSets[i]);
            }

            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &renderSystemLayout->getDescriptorSetLayout();
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;

            pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
        }

        void createPipeline(vk::RenderPass renderPass, HSwapChain& swapchain)
        {
            PipeConf pipelineConfig = PipeConf::createDefault2(swapchain);
            pipelineConfig.renderPass = renderPass;
            pipelineConfig.pipelineLayout = pipelineLayout;
            pipeline = std::make_unique<HPipeline>(device, std::array<HShader, 2>{HShader("../Data/Shaders/vert.spv"), HShader("../Data/Shaders/frag.spv")},
                                                   std::move(pipelineConfig));
        }

        const std::string TEXTURE_PATH = "../Data/Textures/viking_room.png";
        const std::string MODEL_PATH = "../Data/Models/viking_room.obj";

        void loadModel()
        {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
            {
                throw std::runtime_error(warn + err);
            }

            std::unordered_map<Vertex, uint32_t> uniqueVertices{};

            for(const auto& shape: shapes)
            {
                for(const auto& index: shape.mesh.indices)
                {
                    Vertex vertex{};

                    vertex.pos = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.texCoord = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };

                    vertex.color = {1.0f, 1.0f, 1.0f};

                    if(uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }

                    indices.push_back(uniqueVertices[vertex]);
                }
            }
        }

        HDevice& device;
        std::unique_ptr<HPipeline> pipeline;
        vk::PipelineLayout pipelineLayout;
        std::unique_ptr<HDescriptorPool> globalPool;

        std::unique_ptr<HTexture> texture;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::unique_ptr<HBuffer> vertexBuffer;

        std::unique_ptr<HDescriptorSetLayout> renderSystemLayout;
    };

} // Hellion

#endif //HELLION_RENDERSYSTEM_H
