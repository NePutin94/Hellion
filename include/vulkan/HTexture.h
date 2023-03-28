//
// Created by NePutin on 3/19/2023.
//

#ifndef HELLION_HTEXTURE_H
#define HELLION_HTEXTURE_H

#include <vulkan/vulkan.hpp>
#include "HDevice.h"
#include "HBuffer.h"

namespace Hellion
{
    class HTexture
    {
    public:
        HTexture(HDevice& device, const std::string& textureFilepath) : device{device}
        {
            createTextureImage(textureFilepath);
            createTextureImageView(vk::ImageViewType::e2D);
            createTextureSampler();
            updateDescriptor();

        }

        HTexture(HDevice& device, vk::Format format, vk::Extent3D extent, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount) : device{device}
        {
            vk::ImageAspectFlags aspectMask;
            vk::ImageLayout imageLayout;

            format = format;
            extent = extent;

            if(usage & vk::ImageUsageFlagBits::eColorAttachment)
            {
                aspectMask = vk::ImageAspectFlagBits::eColor;
                imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            }
            if(usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
            {
                aspectMask = vk::ImageAspectFlagBits::eDepth;
                imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            }

            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D;
            imageInfo.format = format;
            imageInfo.extent = extent;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = sampleCount;
            imageInfo.tiling = vk::ImageTiling::eOptimal;
            imageInfo.usage = usage;
            imageInfo.initialLayout = vk::ImageLayout::eUndefined;

            auto [img, imgAlloc] = device.createImageWithInfo(imageInfo);
            textureImage = img;
            imageAllocation = imgAlloc;

            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectMask;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            viewInfo.image = textureImage;

            textureImageView = device.getDevice().createImageView(viewInfo);

            if(usage & vk::ImageUsageFlagBits::eSampled)
            {
                auto properties = device.getPhysicalDevice().getProperties();
                vk::SamplerCreateInfo samplerInfo{};
                samplerInfo.magFilter = vk::Filter::eLinear;
                samplerInfo.minFilter = vk::Filter::eLinear;
                samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
                samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
                samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = vk::CompareOp::eAlways;
                samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = static_cast<float>(mipLevels);

                textureSampler = device.getDevice().createSampler(samplerInfo);

                vk::ImageLayout samplerImageLayout = imageLayout == vk::ImageLayout::eColorAttachmentOptimal
                                                   ? vk::ImageLayout::eShaderReadOnlyOptimal
                                                   : vk::ImageLayout::eDepthStencilReadOnlyOptimal;
                descriptor.sampler = textureSampler;
                descriptor.imageView = textureImageView;
                descriptor.imageLayout = samplerImageLayout;
            }
        }

        ~HTexture()
        {
            device.getDevice().destroy(textureSampler);
            device.getDevice().destroy(textureImageView);
            vmaDestroyImage(device.getAllocator(), textureImage, imageAllocation);
        }

        HTexture(const HTexture&) = delete;

        HTexture& operator=(const HTexture&) = delete;

        vk::ImageView imageView() const
        { return textureImageView; }

        vk::Sampler sampler() const
        { return textureSampler; }

        vk::Image getImage() const
        { return textureImage; }

        vk::ImageView getImageView() const
        { return textureImageView; }

        vk::DescriptorImageInfo getImageInfo() const
        { return descriptor; }

        vk::ImageLayout getImageLayout() const
        { return textureLayout; }

        vk::Extent3D getExtent() const
        { return extent; }

        vk::Format getFormat() const
        { return format; }

        void updateDescriptor()
        {
            descriptor.sampler = textureSampler;
            descriptor.imageView = textureImageView;
            descriptor.imageLayout = textureLayout;
        }

        static std::unique_ptr<HTexture> createTextureFromFile(HDevice& device, const std::string& filepath)
        {
            return std::make_unique<HTexture>(device, filepath);
        }

    private:
        void createTextureImage(const std::string& filepath);

        void createTextureImageView(vk::ImageViewType viewType)
        {
            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = textureImage;
            viewInfo.viewType = viewType;
            viewInfo.format = vk::Format::eR8G8B8A8Srgb;
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = layerCount;

            textureImageView = device.getDevice().createImageView(viewInfo);
        }

        void createTextureSampler()
        {
            auto properties = device.getPhysicalDevice().getProperties();

            vk::SamplerCreateInfo samplerInfo{};
            samplerInfo.magFilter = vk::Filter::eLinear;
            samplerInfo.minFilter = vk::Filter::eLinear;
            samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
            samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
            samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = vk::CompareOp::eAlways;
            samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = static_cast<float>(mipLevels);

            textureSampler = device.getDevice().createSampler(samplerInfo);
        }

        vk::DescriptorImageInfo descriptor{};

        VmaAllocation imageAllocation;
        HDevice& device;
        vk::Image textureImage = nullptr;
        vk::ImageView textureImageView = nullptr;
        vk::Sampler textureSampler = nullptr;
        vk::Format format;
        vk::ImageLayout textureLayout;
        uint32_t mipLevels{1};
        uint32_t layerCount{1};
        vk::Extent3D extent{};
    };
}

#endif //HELLION_HTEXTURE_H
