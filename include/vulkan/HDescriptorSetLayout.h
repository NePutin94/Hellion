//
// Created by NePutin on 3/24/2023.
//

#ifndef HELLION_HDESCIPTORSET_H
#define HELLION_HDESCIPTORSET_H

#include <vulkan/vulkan.hpp>
#include <memory>
#include "HDevice.h"
#include <unordered_map>

namespace Hellion
{
    class HDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(HDevice& lveDevice) : lveDevice{lveDevice}
            {}

            Builder& addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32_t count = 1)
            {
                assert(bindings.count(binding) == 0 && "Binding already in use");
                vk::DescriptorSetLayoutBinding layoutBinding{};
                layoutBinding.binding = binding;
                layoutBinding.descriptorType = descriptorType;
                layoutBinding.descriptorCount = count;
                layoutBinding.stageFlags = stageFlags;
                bindings[binding] = layoutBinding;
                return *this;
            }

            std::unique_ptr<HDescriptorSetLayout> build() const
            {
                return std::make_unique<HDescriptorSetLayout>(lveDevice, bindings);
            }

        private:
            HDevice& lveDevice;
            std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
        };

        HDescriptorSetLayout(HDevice& Device, const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>& bindings) : Device{Device}, bindings{bindings}
        {
            std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
            for(auto kv: bindings)
            {
                setLayoutBindings.push_back(kv.second);
            }

            vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
            descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
            descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

            descriptorSetLayout = Device.getDevice().createDescriptorSetLayout(descriptorSetLayoutInfo);
        }

        ~HDescriptorSetLayout()
        {
            Device.getDevice().destroy(descriptorSetLayout);
        }

        HDescriptorSetLayout(const HDescriptorSetLayout&) = delete;

        HDescriptorSetLayout& operator=(const HDescriptorSetLayout&) = delete;

        vk::DescriptorSetLayout getDescriptorSetLayout() const
        { return descriptorSetLayout; }

    private:
        HDevice& Device;
        vk::DescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;

        friend class HDescriptorWriter;
    };

    class HDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(HDevice& lveDevice) : lveDevice{lveDevice}
            {}

            Builder& addPoolSize(vk::DescriptorType descriptorType, uint32_t count)
            {
                poolSizes.emplace_back(descriptorType, count);
                return *this;
            }

            Builder& setPoolFlags(vk::DescriptorPoolCreateFlags flags)
            {
                poolFlags = flags;
                return *this;
            }

            Builder& setMaxSets(uint32_t count)
            {
                maxSets = count;
                return *this;
            }

            std::unique_ptr<HDescriptorPool> build() const
            {
                return std::make_unique<HDescriptorPool>(lveDevice, maxSets, poolFlags, poolSizes);
            }

        private:
            HDevice& lveDevice;
            std::vector<vk::DescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            vk::DescriptorPoolCreateFlags poolFlags = vk::DescriptorPoolCreateFlags{};
        };

        HDescriptorPool(
                HDevice& device,
                uint32_t maxSets,
                vk::DescriptorPoolCreateFlags poolFlags,
                const std::vector<vk::DescriptorPoolSize>& poolSizes)
                : device{device}
        {
            vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
            descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            descriptorPoolInfo.pPoolSizes = poolSizes.data();
            descriptorPoolInfo.maxSets = maxSets;
            descriptorPoolInfo.flags = poolFlags;

            descriptorPool = device.getDevice().createDescriptorPool(descriptorPoolInfo);
        }

        ~HDescriptorPool()
        {
            device.getDevice().destroy(descriptorPool);
        }

        HDescriptorPool(const HDescriptorPool&) = delete;

        HDescriptorPool& operator=(const HDescriptorPool&) = delete;

        bool allocateDescriptor(
                const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet& descriptor) const
        {
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.pSetLayouts = &descriptorSetLayout;
            allocInfo.descriptorSetCount = 1;

            descriptor = device.getDevice().allocateDescriptorSets(allocInfo).back();

            return true;
        }

        void freeDescriptors(std::vector<vk::DescriptorSet>& descriptors) const
        {
            device.getDevice().freeDescriptorSets(descriptorPool, descriptors);
        }

        void resetPool()
        {
            device.getDevice().resetDescriptorPool(descriptorPool);
        }

        HDevice& device;
    private:
        vk::DescriptorPool descriptorPool;

        friend class LveDescriptorWriter;
    };

    class HDescriptorWriter
    {
    public:
        HDescriptorWriter(HDescriptorSetLayout& setLayout, HDescriptorPool& pool)
                : setLayout{setLayout}, pool{pool}
        {}

        HDescriptorWriter& writeBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo)
        {
            assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

            auto& bindingDescription = setLayout.bindings[binding];

            assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

            vk::WriteDescriptorSet write{};
            write.descriptorType = bindingDescription.descriptorType;
            write.dstBinding = binding;
            write.pBufferInfo = bufferInfo;
            write.descriptorCount = 1;

            writes.push_back(write);
            return *this;
        }

        HDescriptorWriter& writeImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo)
        {
            assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

            auto& bindingDescription = setLayout.bindings[binding];

            assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

            vk::WriteDescriptorSet write{};
            write.descriptorType = bindingDescription.descriptorType;
            write.dstBinding = binding;
            write.pImageInfo = imageInfo;
            write.descriptorCount = 1;

            writes.push_back(write);
            return *this;
        }

        bool build(vk::DescriptorSet& set)
        {
            bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
            if(!success)
                return false;
            overwrite(set);
            return true;
        }

        void overwrite(vk::DescriptorSet& set)
        {
            for(auto& write: writes)
                write.dstSet = set;
            pool.device.getDevice().updateDescriptorSets(writes, {});
        }

    private:
        HDescriptorSetLayout& setLayout;
        HDescriptorPool& pool;
        std::vector<vk::WriteDescriptorSet> writes;
    };

} // Hellion

#endif //HELLION_HDESCIPTORSET_H
