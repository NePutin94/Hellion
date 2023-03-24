//
// Created by NePutin on 3/19/2023.
//

#ifndef HELLION_MODELRENDERER_H
#define HELLION_MODELRENDERER_H

#include "RendererBase.h"

namespace Hellion
{
    class ModelRenderer : public RendererBase
    {
    public:
        ModelRenderer(HDevice& vkDev, HellImage depthTexture, const char* modelFile, const char* textureFile, uint32_t uniformDataSize)
                : RendererBase(vkDev, depthTexture)
        {

        }

        ModelRenderer(HDevice& vkDev, HellImage depthTexture, bool useDepth, VkBuffer storageBuffer, VkDeviceMemory storageBufferMemory,
                      uint32_t vertexBufferSize, uint32_t indexBufferSize, HellImage texture, VkSampler textureSampler,
                      const std::vector<const char*>& shaderFiles,
                      uint32_t uniformDataSize, bool useGeneralTextureLayout = true, HellImage externalDepth = {}, bool deleteMeshData = true)
                : RendererBase(vkDev, depthTexture)
        {

        }

        virtual ~ModelRenderer();

        virtual void fillCommandBuffer(VkCommandBuffer commandBuffer, size_t currentImage) override
        {

        }

        void updateUniformBuffer(HDevice& vkDev, uint32_t currentImage, const void* data, const size_t dataSize)
        {

        }

        // HACK to allow sharing textures between multiple ModelRenderers
        void freeTextureSampler()
        { textureSampler_ = VK_NULL_HANDLE; }

    private:
        bool useGeneralTextureLayout_ = false;
        bool isExternalDepth_ = false;
        bool deleteMeshData_ = true;

        size_t vertexBufferSize_;
        size_t indexBufferSize_;

        // 6. Storage Buffer with index and vertex data
        vk::Buffer storageBuffer_;
        VmaAllocation storageBufferAllocation_;

        vk::Sampler textureSampler_;
        HellImage texture_;

        bool createDescriptorSet(HDevice& vkDev, uint32_t uniformDataSize)
        {
            vk::DescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

            vk::DescriptorSetLayoutBinding storage1LayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

            vk::DescriptorSetLayoutBinding storage2LayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

            vk::DescriptorSetLayoutBinding textureLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
            const std::array<vk::DescriptorSetLayoutBinding, 4> bindings = {
                    uboLayoutBinding,
                    storage1LayoutBinding,
                    storage2LayoutBinding,
                    textureLayoutBinding
            };

            vk::DescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.bindingCount = (std::uint32_t) bindings.size();
            layoutInfo.pBindings = bindings.data();

            descriptorSetLayout_ = vkDev.getDevice().createDescriptorSetLayout(layoutInfo);

            std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout_);

            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = descriptorPool_;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts = layouts.data();

            descriptorSets_.resize(MAX_FRAMES_IN_FLIGHT);
            descriptorSets_ = vkDev.getDevice().allocateDescriptorSets(allocInfo);

            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                const vk::DescriptorBufferInfo bufferInfo{uniformBuffers_[i], 0, uniformDataSize};
                const vk::DescriptorBufferInfo bufferInfo2{storageBuffer_, 0, vertexBufferSize_};
                const vk::DescriptorBufferInfo bufferInfo3{storageBuffer_, vertexBufferSize_, indexBufferSize_};
                const vk::DescriptorImageInfo imageInfo{textureSampler_, texture_.imageView,
                                                        useGeneralTextureLayout_ ? vk::ImageLayout::eGeneral : vk::ImageLayout::eShaderReadOnlyOptimal};

                std::array<vk::WriteDescriptorSet, 4> descriptorWrites = {};

                descriptorWrites[0].dstSet = descriptorSets_[i];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &bufferInfo;

                descriptorWrites[1].dstSet = descriptorSets_[i];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pBufferInfo = &bufferInfo2;

                descriptorWrites[2].dstSet = descriptorSets_[i];
                descriptorWrites[2].dstBinding = 2;
                descriptorWrites[2].dstArrayElement = 0;
                descriptorWrites[2].descriptorType = vk::DescriptorType::eStorageBuffer;
                descriptorWrites[2].descriptorCount = 1;
                descriptorWrites[2].pBufferInfo = &bufferInfo3;

                descriptorWrites[3].dstSet = descriptorSets_[i];
                descriptorWrites[3].dstBinding = 3;
                descriptorWrites[3].dstArrayElement = 0;
                descriptorWrites[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
                descriptorWrites[3].descriptorCount = 1;
                descriptorWrites[3].pImageInfo = &imageInfo;

                vkDev.getDevice().updateDescriptorSets(descriptorWrites, {});
            }

            return true;
        }
    };
}

#endif //HELLION_MODELRENDERER_H
