//
// Created by NePutin on 4/6/2023.
//

#ifndef HELLION_HMODEL_H
#define HELLION_HMODEL_H

#include <stdexcept>
#include <fmt/core.h>
#include <tiny_gltf.h>
#include "../include/vulkan/HVertex.h"
#include "vulkan/HDevice.h"
#include "vulkan/HTexture.h"
#include "vulkan/HDescriptorSetLayout.h"

#define MAX_NUM_JOINTS 128u

namespace Hellion
{
    struct Node;

    class BoundingBox
    {
    public:
        glm::vec3 min;
        glm::vec3 max;
        bool valid = false;

        BoundingBox() = default;

        BoundingBox(glm::vec3 min, glm::vec3 max)
                : min(min), max(max)
        {}

        BoundingBox getAABB(glm::mat4 m)
        {
            auto min = glm::vec3(m[3]);
            glm::vec3 max = min;
            glm::vec3 v0, v1;

            auto right = glm::vec3(m[0]);
            v0 = right * this->min.x;
            v1 = right * this->max.x;
            min += glm::min(v0, v1);
            max += glm::max(v0, v1);

            auto up = glm::vec3(m[1]);
            v0 = up * this->min.y;
            v1 = up * this->max.y;
            min += glm::min(v0, v1);
            max += glm::max(v0, v1);

            auto back = glm::vec3(m[2]);
            v0 = back * this->min.z;
            v1 = back * this->max.z;
            min += glm::min(v0, v1);
            max += glm::max(v0, v1);

            return {min, max};
        }
    };

    struct Material
    {

    };

    class Primitive
    {
    private:
        uint32_t firstIndex;
        uint32_t indexCount;
        uint32_t vertexCount;
        // Material& material;
        bool hasIndices;
        BoundingBox bb;
    public:
//        Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material& material) : firstIndex(firstIndex), indexCount(indexCount),
//                                                                                                        vertexCount(vertexCount), material(material)
//        {
//            hasIndices = indexCount > 0;
//        }

        Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount) : firstIndex(firstIndex), indexCount(indexCount),
                                                                                    vertexCount(vertexCount)
        {
            hasIndices = indexCount > 0;
        }

        void setBoundingBox(glm::vec3 min, glm::vec3 max)
        {
            bb.min = min;
            bb.max = max;
            bb.valid = true;
        }

        uint32_t getFirstIndex() const
        {
            return firstIndex;
        }

        void setFirstIndex(uint32_t firstIndex)
        {
            Primitive::firstIndex = firstIndex;
        }

        uint32_t getIndexCount() const
        {
            return indexCount;
        }

        void setIndexCount(uint32_t indexCount)
        {
            Primitive::indexCount = indexCount;
        }

        uint32_t getVertexCount() const
        {
            return vertexCount;
        }

        void setVertexCount(uint32_t vertexCount)
        {
            Primitive::vertexCount = vertexCount;
        }

        bool isHasIndices() const
        {
            return hasIndices;
        }

        void setHasIndices(bool hasIndices)
        {
            Primitive::hasIndices = hasIndices;
        }

        const BoundingBox& getBb() const
        {
            return bb;
        }

        void setBb(const BoundingBox& bb)
        {
            Primitive::bb = bb;
        }
    };

    class Mesh
    {
    private:
        HDevice& device;
        std::vector<Primitive> primitives;
        BoundingBox bb;
        BoundingBox aabb;
        vk::DescriptorSet meshDescriptorSets;
        std::unique_ptr<HBuffer> uniformBuffer;

    public:

        struct UniformBlock
        {
            glm::mat4 matrix;
            glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
            float jointcount{0};
        } uniformBlock;

        HBuffer* getUniformBuffer()
        {
            return uniformBuffer.get();
        }

        vk::DescriptorSet& getDescriptorSet()
        {
            return meshDescriptorSets;
        }

        Mesh(HDevice& device, glm::mat4 matrix) : device(device)
        {
            uniformBuffer = std::make_unique<HBuffer>(
                    device,
                    sizeof(uniformBlock),
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                    VMA_ALLOCATION_CREATE_MAPPED_BIT);
        }

        const std::vector<Primitive>& getPrimitives() const
        {
            return primitives;
        }

        std::vector<Primitive>& getPrimitives()
        {
            return primitives;
        }

        void setPrimitives(const std::vector<Primitive>& primitives)
        {
            Mesh::primitives = primitives;
        }

        const BoundingBox& getBb() const
        {
            return bb;
        }

        BoundingBox& getBb()
        {
            return bb;
        }

        void setBb(const BoundingBox& bb)
        {
            Mesh::bb = bb;
        }

        ~Mesh()
        {

        }

        void setBoundingBox(glm::vec3 min, glm::vec3 max)
        {
            bb.min = min;
            bb.max = max;
            bb.valid = true;
        }
    };

    struct Skin
    {
        std::string name;
        Node* skeletonRoot = nullptr;
        std::vector<glm::mat4> inverseBindMatrices;
        std::vector<Node*> joints;
    };

    class Node
    {
    public:
        Node* parent;
        uint32_t index;
        std::vector<Node*> children;
        glm::mat4 matrix;
        std::string name;
        Mesh* mesh;
        Skin* skin;
        int32_t skinIndex = -1;
        glm::vec3 translation{};
        glm::vec3 scale{1.0f};
        glm::quat rotation{};
        BoundingBox bvh;
        BoundingBox aabb;

        glm::mat4 localMatrix()
        {
            return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
        }

        glm::mat4 getMatrix()
        {
            glm::mat4 m = localMatrix();
            Node* p = parent;
            while(p)
            {
                m = p->localMatrix() * m;
                p = p->parent;
            }
            return m;
        }

        void update()
        {
            if(mesh)
            {
                glm::mat4 m = getMatrix();
                if(skin)
                {
                    mesh->uniformBlock.matrix = m;
                    // Update join matrices
                    glm::mat4 inverseTransform = glm::inverse(m);
                    size_t numJoints = std::min((uint32_t) skin->joints.size(), MAX_NUM_JOINTS);
                    for(size_t i = 0; i < numJoints; i++)
                    {
                        Node* jointNode = skin->joints[i];
                        glm::mat4 jointMat = jointNode->getMatrix() * skin->inverseBindMatrices[i];
                        jointMat = inverseTransform * jointMat;
                        mesh->uniformBlock.jointMatrix[i] = jointMat;
                    }
                    mesh->uniformBlock.jointcount = (float) numJoints;
                    memcpy(mesh->getUniformBuffer()->getMappedMemory(), &mesh->uniformBlock, sizeof(mesh->uniformBlock));
                } else
                {
                    memcpy(mesh->getUniformBuffer()->getMappedMemory(), &m, sizeof(glm::mat4));
                }
            }

            for(auto& child: children)
            {
                child->update();
            }
        }

        static void setupNodeDescriptorSet(Node* node, HDescriptorPool& pool, HDevice& device, HDescriptorSetLayout& meshLayout)
        {
            if(node->mesh)
            {
                auto bufferInfo = node->mesh->getUniformBuffer()->descriptorInfo();
                HDescriptorWriter(meshLayout, pool)
                        .writeBuffer(0, &bufferInfo)
                        .build(node->mesh->getDescriptorSet());
            }
            for(auto& child: node->children)
            {
                setupNodeDescriptorSet(child, pool, device, meshLayout);
            }
        }

    };

    class HModel
    {
    public:
        HModel(HDevice& device) : device(device)
        {}

        struct LoaderInfo
        {
            std::vector<uint32_t> indexBuffer;
            std::vector<HVertex> vertexBuffer;
            size_t indexPos = 0;
            size_t vertexPos = 0;
        };

        std::unique_ptr<HBuffer> vertices;
        std::unique_ptr<HBuffer> indices;
        LoaderInfo loaderInfo{};

        void getNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount);

        void loadFromFile(std::string path);

        void loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, float globalscale,
                      LoaderInfo& loaderInfo);

        void createVertexBufferVma(vk::DeviceSize bufferSize)
        {
            HELLION_ZONE_PROFILING()
            auto stagingBuffer = HBuffer(device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);

            memcpy(stagingBuffer.getMappedMemory(), loaderInfo.vertexBuffer.data(), (size_t) bufferSize);
            vertices = std::make_unique<HBuffer>(device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, 0);

            device.copyBuffer(stagingBuffer.getBuffer(), vertices->getBuffer(), bufferSize);
        }

        void createIndexBufferVma(vk::DeviceSize bufferSize)
        {
            HELLION_ZONE_PROFILING()
            auto stagingBuffer = HBuffer(device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                                                    VMA_ALLOCATION_CREATE_MAPPED_BIT);

            memcpy(stagingBuffer.getMappedMemory(), loaderInfo.indexBuffer.data(), (size_t) bufferSize);
            indices = std::make_unique<HBuffer>(device, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, 0);

            device.copyBuffer(stagingBuffer.getBuffer(), indices->getBuffer(), bufferSize);
        }

        void draw(vk::CommandBuffer& buffer)
        {
            const VkDeviceSize offsets[1] = {0};
            vk::Buffer vertexBuffers[] = {vertices->getBuffer()};
            buffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
            buffer.bindIndexBuffer(indices->getBuffer(), 0, vk::IndexType::eUint32);
            for(auto& node: nodes)
            {
                drawNode(node, buffer);
            }
        }

        void drawNode(Node* node, vk::CommandBuffer& buffer)
        {
            if(node->mesh)
            {
                for(auto& primitive: node->mesh->getPrimitives())
                {
                    buffer.drawIndexed(primitive.getIndexCount(), 1, primitive.getFirstIndex(), 0, 0);
                }
            }
            for(auto& child: node->children)
            {
                drawNode(child, buffer);
            }
        }

        std::vector<Node*>& getNodes();

        size_t getMeshCount()
        {
            size_t meshCount = 0;
            for(auto node: linearNodes)
            {
                if(node->mesh)
                {
                    meshCount++;
                }
            }
            return meshCount;
        }

    private:
        HDevice& device;

        glm::mat4 aabb;
        std::vector<Node*> nodes;
        std::vector<Node*> linearNodes;
        std::vector<Skin*> skins;
        std::vector<HTexture> textures;
        // std::vector<TextureSampler> textureSamplers;
        std::vector<Material> materials;
        //std::vector<Animation> animations;
        std::vector<std::string> extensions;
    };

} // Hellion



#endif //HELLION_HMODEL_H
