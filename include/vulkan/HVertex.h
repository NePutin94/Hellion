//
// Created by NePutin on 3/24/2023.
//

#ifndef HELLION_HVERTEX_H
#define HELLION_HVERTEX_H

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace Hellion
{
    struct HVertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        HVertex() = default;

        static vk::VertexInputBindingDescription getBindingDescriptions()
        {
            vk::VertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(HVertex);
            bindingDescription.inputRate = vk::VertexInputRate::eVertex;

            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[0].offset = offsetof(HVertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[1].offset = offsetof(HVertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[2].offset = offsetof(HVertex, texCoord);

            return attributeDescriptions;
        }

        bool operator==(const HVertex& other) const
        {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };

    struct HVertexLine
    {
        glm::vec3 pos;
        glm::vec3 color;

        static vk::VertexInputBindingDescription getBindingDescriptions()
        {
            vk::VertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(HVertexLine);
            bindingDescription.inputRate = vk::VertexInputRate::eVertex;

            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[0].offset = offsetof(HVertexLine, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
            attributeDescriptions[1].offset = offsetof(HVertexLine, color);
            return attributeDescriptions;
        }

        bool operator==(const HVertexLine& other) const
        {
            return pos == other.pos && color == other.color;
        }
    };
} // Hellion

namespace std
{
    template<>
    struct hash<Hellion::HVertex>
    {
        size_t operator()(const Hellion::HVertex& x) const
        {
            std::size_t h1 = std::hash<glm::vec3>{}(x.pos);
            std::size_t h2 = std::hash<glm::vec3>{}(x.color);
            std::size_t h3 = std::hash<glm::vec2>{}(x.texCoord);
            size_t seed = 0;
            seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}

#endif //HELLION_HVERTEX_H
