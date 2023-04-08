//
// Created by NePutin on 3/28/2023.
//

#include "../../include/vulkan/RenderSystem.h"

void Hellion::RenderSystem::draw(vk::CommandBuffer& buffer, uint32_t currentFrame, tracy::VkCtx* tracyCtx)
{
    HELLION_ZONE_PROFILING()
    HELLION_GPUZONE_PROFILING(tracyCtx, buffer, "RenderSystem draw")
    pipeline->bind(buffer);

//    vk::Buffer vertexBuffers[] = {model.vertices->getBuffer()};
//
//    vk::DeviceSize offsets[] = {0};
//
//    buffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
//
//    buffer.bindIndexBuffer(model.indices->getBuffer(), 0, vk::IndexType::eUint32);
//
//    for(auto& node : model.getNodes())
//    {
//        renderNode(buffer, currentFrame, tracyCtx, node);
//    }

    const VkDeviceSize offsets[1] = {0};
    vk::Buffer vertexBuffers[] = {model.vertices->getBuffer()};
    buffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    buffer.bindIndexBuffer(model.indices->getBuffer(), 0, vk::IndexType::eUint32);
   // buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &globalDescriptorSets[currentFrame], 0, nullptr);
    for(auto& node : model.getNodes())
    {
        renderNode(buffer, currentFrame, tracyCtx, node);
    }

//    vk::Buffer vertexBuffers[] = {vertexBuffer->getBuffer()};
//
//    vk::DeviceSize offsets[] = {0};
//
//    buffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
//
//    buffer.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);

    //model.draw(buffer);

   // buffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}