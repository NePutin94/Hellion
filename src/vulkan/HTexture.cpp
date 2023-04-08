//
// Created by NePutin on 3/19/2023.
//

#include "../../include/vulkan/HTexture.h"

#include <stb_image.h>

void Hellion::HTexture::createTextureImage(const std::string& filepath)
{
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels =
            stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    if(!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    // mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    mipLevels = 1;

    HBuffer stagingBuffer(device, imageSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);

    memcpy(stagingBuffer.getMappedMemory(), pixels, (size_t) imageSize);
    stbi_image_free(pixels);


    format = vk::Format::eR8G8B8A8Srgb;
    extent = vk::Extent3D{static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1};

    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent = extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    auto [img, imgAlloc] = device.createImageWithInfo(imageInfo);
    textureImage = img;
    imageAllocation = imgAlloc;

    device.transitionImageLayout(
            textureImage,
            vk::Format::eR8G8B8A8Srgb,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal,
            mipLevels,
            layerCount);
    device.copyBufferToImage(
            stagingBuffer.getBuffer(),
            textureImage,
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight));

    device.transitionImageLayout(
            textureImage,
            vk::Format::eR8G8B8A8Srgb,
            vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            mipLevels,
            layerCount);

    textureLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}
