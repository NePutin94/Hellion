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
#include <chrono>
#include <tracy/TracyVulkan.hpp>
#include "../core/Profiling.h"
#include "../HCamera.h"
#include "../HModel.h"

namespace Hellion
{
    class RenderSystem
    {
    public:
        RenderSystem(HDevice& device, vk::RenderPass renderPass, HSwapChain& swapchain)
                : device{device}, model{device}
        {
            loadModel();
            createPipelineLayout();
            createPipeline(renderPass, swapchain);
            //createVertexBufferVma();
            //createIndexBufferVma();
        }

        ~RenderSystem()
        {
            device.getDevice().destroy(pipelineLayout);
        }

        RenderSystem(const RenderSystem&) = delete;

        RenderSystem& operator=(const RenderSystem&) = delete;

        void draw(vk::CommandBuffer& buffer, uint32_t currentFrame, tracy::VkCtx* tracyCtx);

        void updateBuffers(uint32_t currentFrame, float width, float height, HCamera camera)
        {
            HELLION_ZONE_PROFILING()
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            UniformBufferObject ubo{};
            ubo.model = glm::mat4 {1};
            ubo.view = camera.getViewMatrix();//glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = camera.getProjectionMatrix();//glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
            ubo.proj[1][1] *= -1;
            ubo.time = time * 15;
            memcpy(uboBuffers[currentFrame]->getMappedMemory(), &ubo, sizeof(ubo));
        }

        void renderNode(vk::CommandBuffer& buffer, uint32_t currentFrame, tracy::VkCtx* tracyCtx, Node* node)
        {
            if(node->mesh)
            {
                for(auto& primitive: node->mesh->getPrimitives())
                {
                    const std::vector<vk::DescriptorSet> descriptorsets = {
                            globalDescriptorSets[currentFrame],
                            node->mesh->getDescriptorSet(),
                    };
                    buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorsets, {});
                    buffer.drawIndexed(primitive.getIndexCount(), 1, primitive.getFirstIndex(), 0, 0);
                }
            }
            for(auto& child: node->children)
            {
                renderNode(buffer, currentFrame, tracyCtx, child);
            }

//            if(node->mesh)
//            {
//                // Render mesh primitives
//                for(auto primitive: node->mesh->getPrimitives())
//                {
//                    const std::vector<vk::DescriptorSet> descriptorsets = {
//                            globalDescriptorSets[currentFrame],
//                            node->mesh->getDescriptorSet(),
//                    };
//
//                    buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorsets, {});
//
//                    if(primitive.isHasIndices())
//                    {
//                        buffer.drawIndexed(primitive.getIndexCount(), 1, primitive.getFirstIndex(), 0, 0);
//                    } else
//                    {
//                        buffer.draw(primitive.getIndexCount(), primitive.getVertexCount(), 1, 0);
//                    }
//                }
//            }
//            for(auto& child: node->children)
//            {
//                renderNode(buffer, currentFrame, tracyCtx, child);
//            }
        }

    private:
        struct UniformBufferObject
        {
            alignas(16) glm::mat4 model;
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 proj;
            alignas(16) float time;
        };
        HModel model;


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

        void createIndexBufferVma()
        {
            HELLION_ZONE_PROFILING()
            vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();
            auto stagingBuffer = HBuffer(device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);

            memcpy(stagingBuffer.getMappedMemory(), indices.data(), (size_t) bufferSize);
            indexBuffer = std::make_unique<HBuffer>(device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, 0);

            device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
        }

        void createPipelineLayout()
        {
            HELLION_ZONE_PROFILING()
            texture = HTexture::createTextureFromFile(device, TEXTURE_PATH.c_str());

            size_t meshCount = model.getMeshCount();

            globalPool =
                    HDescriptorPool::Builder(device)
                            .setMaxSets(HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .addPoolSize(vk::DescriptorType::eUniformBuffer, HSwapChain::MAX_FRAMES_IN_FLIGHT)
                                    // .addPoolSize(vk::DescriptorType::eCombinedImageSampler, HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .build();
            meshPool =
                    HDescriptorPool::Builder(device)
                            .setMaxSets(meshCount * HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .addPoolSize(vk::DescriptorType::eUniformBuffer, meshCount * HSwapChain::MAX_FRAMES_IN_FLIGHT)
                                    // .addPoolSize(vk::DescriptorType::eCombinedImageSampler, HSwapChain::MAX_FRAMES_IN_FLIGHT)
                            .build();

            uboBuffers.resize(HSwapChain::MAX_FRAMES_IN_FLIGHT);

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


            meshLayout =
                    HDescriptorSetLayout::Builder(device)
                            .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                            .build();

            for(auto& node: model.getNodes())
            {
                Node::setupNodeDescriptorSet(node, *meshPool, device, *meshLayout);
            }

            globalDescriptorSets.resize(HSwapChain::MAX_FRAMES_IN_FLIGHT);
            for(int i = 0; i < globalDescriptorSets.size(); i++)
            {
                auto imageInfo = texture->getImageInfo();
                auto bufferInfo = uboBuffers[i]->descriptorInfo();
                HDescriptorWriter(*renderSystemLayout, *globalPool)
                        .writeBuffer(0, &bufferInfo)
                        .writeImage(1, &imageInfo)
                        .build(globalDescriptorSets[i]);
            }

            const std::vector<vk::DescriptorSetLayout> setLayouts = {
                    renderSystemLayout->getDescriptorSetLayout(), meshLayout->getDescriptorSetLayout()
            };

            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.setLayoutCount = setLayouts.size();
            pipelineLayoutInfo.pSetLayouts = setLayouts.data();
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
            HELLION_ZONE_PROFILING()
            model.loadFromFile("../Data/Models/gLTF/SimpleMeshes.gltf");
//            tinyobj::attrib_t attrib;
//            std::vector<tinyobj::shape_t> shapes;
//            std::vector<tinyobj::material_t> materials;
//            std::string warn, err;
//
//            if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
//            {
//                throw std::runtime_error(warn + err);
//            }
//
//            std::unordered_map<HVertex, uint32_t> uniqueVertices{};
//
//            for(const auto& shape: shapes)
//            {
//                for(const auto& index: shape.mesh.indices)
//                {
//                    HVertex vertex{};
//
//                    vertex.pos = {
//                            attrib.vertices[3 * index.vertex_index + 0],
//                            attrib.vertices[3 * index.vertex_index + 1],
//                            attrib.vertices[3 * index.vertex_index + 2]
//                    };
//
//                    vertex.texCoord = {
//                            attrib.texcoords[2 * index.texcoord_index + 0],
//                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
//                    };
//
//                    vertex.color = {1.0f, 1.0f, 1.0f};
//
//                    if(uniqueVertices.count(vertex) == 0)
//                    {
//                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
//                        vertices.push_back(vertex);
//                    }
//
//                    indices.push_back(uniqueVertices[vertex]);
//                }
//            }
        }

        HDevice& device;
        std::unique_ptr<HPipeline> pipeline;
        vk::PipelineLayout pipelineLayout;
        std::unique_ptr<HDescriptorPool> meshPool;
        std::unique_ptr<HDescriptorPool> globalPool;
        std::unique_ptr<HTexture> texture;

        std::vector<std::unique_ptr<HBuffer>> uboBuffers;

        std::vector<HVertex> vertices;
        std::vector<uint32_t> indices;
        std::unique_ptr<HBuffer> vertexBuffer;
        std::unique_ptr<HBuffer> indexBuffer;
        std::vector<vk::DescriptorSet> globalDescriptorSets;
        std::unique_ptr<HDescriptorSetLayout> renderSystemLayout;
        std::unique_ptr<HDescriptorSetLayout> meshLayout;
    };

} // Hellion

#endif //HELLION_RENDERSYSTEM_H
