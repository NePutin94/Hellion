//
// Created by NePutin on 4/6/2023.
//

#include "../include/HModel.h"
#include "glm/vec3.hpp"


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <tiny_gltf.h>
#include <glm/gtc/type_ptr.hpp>

void Hellion::HModel::loadFromFile(std::string path)
{
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error;
    std::string warn;

    size_t vertexCount = 0;
    size_t indexCount = 0;

    bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warn, path);
    if(fileLoaded)
    {
        const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
        for(size_t i = 0; i < scene.nodes.size(); i++)
            getNodeProps(gltfModel.nodes[scene.nodes[i]], gltfModel, vertexCount, indexCount);

        loaderInfo.vertexBuffer.resize(vertexCount);
        loaderInfo.indexBuffer.resize(indexCount);

        for(size_t i = 0; i < scene.nodes.size(); i++)
        {
            const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
            loadNode(nullptr, node, scene.nodes[i], gltfModel, 1.f, loaderInfo);
        }

        for (auto node : linearNodes) {
            if (node->skinIndex > -1) {
                node->skin = skins[node->skinIndex];
            }
            if (node->mesh) {
                node->update();
            }
        }

    } else
        throw std::runtime_error(fmt::format("Could not load gltf file: {}", path));

    size_t vertexBufferSize = vertexCount * sizeof(HVertex);
    size_t indexBufferSize = indexCount * sizeof(uint32_t);

    assert(vertexBufferSize > 0);

    createVertexBufferVma(vertexBufferSize);
    createIndexBufferVma(indexBufferSize);
}

void Hellion::HModel::getNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount)
{
    if(node.children.size() > 0)
    {
        for(size_t i = 0; i < node.children.size(); i++)
        {
            getNodeProps(model.nodes[node.children[i]], model, vertexCount, indexCount);
        }
    }
    if(node.mesh > -1)
    {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        for(size_t i = 0; i < mesh.primitives.size(); i++)
        {
            auto primitive = mesh.primitives[i];
            vertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;
            if(primitive.indices > -1)
            {
                indexCount += model.accessors[primitive.indices].count;
            }
        }
    }
}

void Hellion::HModel::loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, float globalscale,
                               LoaderInfo& loaderInfo)
{
    auto newNode = new Node();
    newNode->index = nodeIndex;
    newNode->parent = parent;
    newNode->name = node.name;
    newNode->skinIndex = node.skin;
    newNode->matrix = glm::mat4(1.0f);

    // Generate local node matrix
    glm::vec3 translation = glm::vec3(0.0f);
    if(node.translation.size() == 3)
    {
        translation = glm::make_vec3(node.translation.data());
        newNode->translation = translation;
    }
    glm::mat4 rotation = glm::mat4(1.0f);
    if(node.rotation.size() == 4)
    {
        glm::quat q = glm::make_quat(node.rotation.data());
        newNode->rotation = glm::mat4(q);
    }
    glm::vec3 scale = glm::vec3(1.0f);
    if(node.scale.size() == 3)
    {
        scale = glm::make_vec3(node.scale.data());
        newNode->scale = scale;
    }
    if(node.matrix.size() == 16)
    {
        newNode->matrix = glm::make_mat4x4(node.matrix.data());
    }

    // Node with children
    if(node.children.size() > 0)
    {
        for(size_t i = 0; i < node.children.size(); i++)
        {
            loadNode(newNode, model.nodes[node.children[i]], node.children[i], model, globalscale, loaderInfo);
        }
    }

    // Node contains mesh data
    if(node.mesh > -1)
    {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        auto newMesh = new Mesh(device, newNode->matrix);
        for(size_t j = 0; j < mesh.primitives.size(); j++)
        {
            const tinygltf::Primitive& primitive = mesh.primitives[j];
            uint32_t vertexStart = static_cast<uint32_t>(loaderInfo.vertexPos);
            uint32_t indexStart = static_cast<uint32_t>(loaderInfo.indexPos);
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            glm::vec3 posMin{};
            glm::vec3 posMax{};
            bool hasSkin = false;
            bool hasIndices = primitive.indices > -1;
            // Vertices
            {
                const float* bufferPos = nullptr;
                const float* bufferNormals = nullptr;
                const float* bufferTexCoordSet0 = nullptr;
                const float* bufferTexCoordSet1 = nullptr;
                const float* bufferColorSet0 = nullptr;
                const void* bufferJoints = nullptr;
                const float* bufferWeights = nullptr;

                int posByteStride;
                int normByteStride;
                int uv0ByteStride;
                int uv1ByteStride;
                int color0ByteStride;
                int jointByteStride;
                int weightByteStride;

                int jointComponentType;

                // Position attribute is required
                assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

                const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
                bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
                posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
                posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
                vertexCount = static_cast<uint32_t>(posAccessor.count);
                posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(
                        TINYGLTF_TYPE_VEC3);

                if(primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
                    bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
                    normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(
                            TINYGLTF_TYPE_VEC3);
                }

                // UVs
                if(primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
                    bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                    uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(
                            TINYGLTF_TYPE_VEC2);
                }
                if(primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
                {
                    const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
                    const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
                    bufferTexCoordSet1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                    uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(
                            TINYGLTF_TYPE_VEC2);
                }

                // Vertex colors
                if(primitive.attributes.find("COLOR_0") != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    bufferColorSet0 = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(
                            TINYGLTF_TYPE_VEC3);
                }

                // Skinning
                // Joints
                if(primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
                {
                    const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
                    const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
                    bufferJoints = &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
                    jointComponentType = jointAccessor.componentType;
                    jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) /
                                                                             tinygltf::GetComponentSizeInBytes(jointComponentType))
                                                                          : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
                }

                if(primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
                {
                    const tinygltf::Accessor& weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
                    const tinygltf::BufferView& weightView = model.bufferViews[weightAccessor.bufferView];
                    bufferWeights = reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
                    weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float))
                                                                             : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
                }

                hasSkin = (bufferJoints && bufferWeights);

                for(size_t v = 0; v < posAccessor.count; v++)
                {
                    HVertex& vert = loaderInfo.vertexBuffer[loaderInfo.vertexPos];
                    vert.pos = glm::make_vec3(&bufferPos[v * posByteStride]);
                    vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
                    vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
                    vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);
                    vert.color = bufferColorSet0 ? glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) : glm::vec4(1.0f);

                    if(hasSkin)
                    {
                        switch(jointComponentType)
                        {
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            {
                                const uint16_t* buf = static_cast<const uint16_t*>(bufferJoints);
                                vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
                                break;
                            }
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            {
                                const uint8_t* buf = static_cast<const uint8_t*>(bufferJoints);
                                vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
                                break;
                            }
                            default:
                                // Not supported by spec
                                //std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
                                break;
                        }
                    } else
                    {
                        vert.joint0 = glm::vec4(0.0f);
                    }
                    vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
                    // Fix for all zero weights
                    if(glm::length(vert.weight0) == 0.0f)
                    {
                        vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
                    }
                    loaderInfo.vertexPos++;
                }
            }
            // Indices
            if(hasIndices)
            {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);
                const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

                switch(accessor.componentType)
                {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
                        for(size_t index = 0; index < accessor.count; index++)
                        {
                            loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
                            loaderInfo.indexPos++;
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
                        for(size_t index = 0; index < accessor.count; index++)
                        {
                            loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
                            loaderInfo.indexPos++;
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
                        for(size_t index = 0; index < accessor.count; index++)
                        {
                            loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
                            loaderInfo.indexPos++;
                        }
                        break;
                    }
                    default:
                        //std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                        return;
                }
            }
            Primitive newPrimitive(indexStart, indexCount, vertexCount);
            newPrimitive.setBoundingBox(posMin, posMax);
            newMesh->getPrimitives().push_back(newPrimitive);
        }
        // Mesh BB from BBs of primitives
        for(auto p: newMesh->getPrimitives())
        {
            if(p.getBb().valid && !newMesh->getBb().valid)
            {
                newMesh->getBb() = p.getBb();
                newMesh->getBb().valid = true;
            }
            newMesh->getBb().min = glm::min(newMesh->getBb().min, p.getBb().min);
            newMesh->getBb().max = glm::max(newMesh->getBb().max, p.getBb().max);
        }
        newNode->mesh = newMesh;
    }
    if(parent)
    {
        parent->children.push_back(newNode);
    } else
    {
        nodes.push_back(newNode);
    }
    linearNodes.push_back(newNode);
}

std::vector<Hellion::Node*>& Hellion::HModel::getNodes()
{
    return nodes;
}
