//
// Created by NePutin on 3/19/2023.
//

#ifndef HELLION_HTEXTURE_H
#define HELLION_HTEXTURE_H

#include <vulkan/vulkan.hpp>

namespace Hellion
{
    class HTexture
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        vk::Format format;

        vk::Image image;
        vk::Sampler sampler;

        // Offscreen buffers require VK_IMAGE_LAYOUT_GENERAL && static textures have VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        vk::ImageLayout desiredLayout;
    };
}

#endif //HELLION_HTEXTURE_H
