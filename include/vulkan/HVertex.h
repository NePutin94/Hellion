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
        glm::vec4 color;
        glm::vec2 texCoord;
        glm::vec3 normal;
        glm::vec2 uv0;
        glm::vec2 uv1;
        glm::vec4 joint0;
        glm::vec4 weight0;
        HVertex() = default;

        static vk::VertexInputBindingDescription getBindingDescriptions()
        {
            vk::VertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(HVertex);
            bindingDescription.inputRate = vk::VertexInputRate::eVertex;

            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 7> getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 7> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[0].offset = offsetof(HVertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[1].offset = offsetof(HVertex, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[2].offset = offsetof(HVertex, uv0);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[3].offset = offsetof(HVertex, uv1);

            attributeDescriptions[4].binding = 0;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = vk::Format::eR32G32B32A32Sfloat;
            attributeDescriptions[4].offset = offsetof(HVertex, joint0);

            attributeDescriptions[5].binding = 0;
            attributeDescriptions[5].location = 5;
            attributeDescriptions[5].format = vk::Format::eR32G32B32A32Sfloat;
            attributeDescriptions[5].offset = offsetof(HVertex, weight0);

            attributeDescriptions[6].binding = 0;
            attributeDescriptions[6].location = 6;
            attributeDescriptions[6].format = vk::Format::eR32G32B32A32Sfloat;
            attributeDescriptions[6].offset = offsetof(HVertex, color);

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
    template<typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest& ... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    }

    template<>
    struct hash<Hellion::HVertex>
    {
        size_t operator()(const Hellion::HVertex& x) const
        {
            size_t seed = 0;
            hashCombine(seed, x.pos, x.color, x.texCoord);
            return seed;
        }
    };
}

#endif //HELLION_HVERTEX_H
