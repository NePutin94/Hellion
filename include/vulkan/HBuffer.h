//
// Created by NePutin on 3/24/2023.
//

#ifndef HELLION_HBUFFER_H
#define HELLION_HBUFFER_H

#include <vulkan/vulkan.hpp>
#include "HDevice.h"

namespace Hellion
{

    class HBuffer
    {
    public:
        HBuffer(
                HDevice& device,
                vk::DeviceSize bufferSize,
                vk::BufferUsageFlags usageFlags,
                VmaAllocationCreateFlags memoryPropertyFlags,
                vk::DeviceSize minOffsetAlignment = 1) : device{device},
                                                         bufferSize{bufferSize},
                                                         usageFlags{usageFlags},
                                                         memoryPropertyFlags{memoryPropertyFlags}
        {
            auto [vmaAlloc, vmaAllocInfo] = device.createBufferVma(bufferSize, usageFlags, buffer, memoryPropertyFlags);
            allocation = vmaAlloc;
            info = vmaAllocInfo;
        }

        ~HBuffer()
        {
            unmap();
//            vkDestroyBuffer(lveDevice.device(), buffer, nullptr);
//            vkFreeMemory(lveDevice.device(), memory, nullptr);
        }

        HBuffer(const HBuffer&) = delete;

        HBuffer& operator=(const HBuffer&) = delete;

        void* map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0)
        {
            void* mappedData;
            vmaMapMemory(device.getAllocator(), allocation, &mappedData);
            return mappedData;
        }

        void unmap()
        { vmaUnmapMemory(device.getAllocator(), allocation); }

        void writeToBuffer(void* data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0)
        {
            assert(mapped && "Cannot copy to unmapped buffer");

            if(size == VK_WHOLE_SIZE)
            {
                memcpy(mapped, data, bufferSize);
            } else
            {
                char* memOffset = (char*) mapped;
                memOffset += offset;
                memcpy(memOffset, data, size);
            }
        }

        VkResult flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0)
        { vmaFlushAllocation(device.getAllocator(), allocation, offset, size); }

        vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0)
        {
            return vk::DescriptorBufferInfo{
                    buffer,
                    offset,
                    size,
            };
        }

        VkResult invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0)
        { vmaInvalidateAllocation(device.getAllocator(), allocation, offset, size); }

        vk::Buffer getBuffer() const
        { return buffer; }

        void* getMappedMemory() const
        { return mapped; }


        vk::BufferUsageFlags getUsageFlags() const
        { return usageFlags; }

        VmaAllocationCreateFlags getMemoryPropertyFlags() const
        { return memoryPropertyFlags; }

        vk::DeviceSize getBufferSize() const
        { return bufferSize; }

    private:
        static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment)
        {
            if(minOffsetAlignment > 0)
            {
                return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
            }
            return instanceSize;
        }

        HDevice& device;
        void* mapped = nullptr;
        vk::Buffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation;
        VmaAllocationInfo info;

        vk::DeviceSize bufferSize;
        vk::BufferUsageFlags usageFlags;
        VmaAllocationCreateFlags memoryPropertyFlags;
    };

} // Hellion

#endif //HELLION_HBUFFER_H
