//
// Created by NePutin on 4/2/2023.
//

#include "../../include/vulkan/CanvasSystem.h"


void Hellion::CanvasSystem::draw(vk::CommandBuffer& buffer, uint32_t currentFrame, tracy::VkCtx* tracyCtx)
{
    HELLION_ZONE_PROFILING()
    HELLION_GPUZONE_PROFILING(tracyCtx, buffer, "Canvas draw")
    pipeline->bind(buffer);

    buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &globalDescriptorSets[currentFrame], 0, nullptr);

    vk::Buffer vertexBuffers[] = {vertexBuffer->getBuffer()};

    vk::DeviceSize offsets[] = {0};

    buffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

    buffer.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0 );
}

void Hellion::CanvasSystem::line(glm::vec3 p1, glm::vec3 p2, glm::vec4 c)
{
    vertices.push_back( HVertexLine{ .pos = p1, .color = c } );
    vertices.push_back( HVertexLine{ .pos = p2, .color = c } );
}

void Hellion::CanvasSystem::plane3d(const glm::vec3& o, const glm::vec3& v1, const glm::vec3& v2, int n1, int n2, float s1, float s2, const glm::vec4& color,
                                    const glm::vec4& outlineColor)
{
    line(o - s1 / 2.0f * v1 - s2 / 2.0f * v2, o - s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
    line(o + s1 / 2.0f * v1 - s2 / 2.0f * v2, o + s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);

    line(o - s1 / 2.0f * v1 + s2 / 2.0f * v2, o + s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
    line(o - s1 / 2.0f * v1 - s2 / 2.0f * v2, o + s1 / 2.0f * v1 - s2 / 2.0f * v2, outlineColor);

    for (int i = 1; i < n1; i++)
    {
        float t = ((float)i - (float)n1 / 2.0f) * s1 / (float)n1;
        const glm::vec3 o1 = o + t * v1;
        line(o1 - s2 / 2.0f * v2, o1 + s2 / 2.0f * v2, color);
    }

    for (int i = 1; i < n2; i++)
    {
        const float t = ((float)i - (float)n2 / 2.0f) * s2 / (float)n2;
        const glm::vec3 o2 = o + t * v2;
        line(o2 - s1 / 2.0f * v1, o2 + s1 / 2.0f * v1, color);
    }
}
