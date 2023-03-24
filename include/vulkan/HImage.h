//
// Created by NePutin on 3/19/2023.
//

#ifndef HELLION_HIMAGE_H
#define HELLION_HIMAGE_H
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace Hellion
{
    class HellImage
    {
    public:
        vk::Image image = nullptr;
        vk::ImageView imageView = nullptr;
        VmaAllocation imageAllocation;
    };
}

#endif //HELLION_HIMAGE_H
