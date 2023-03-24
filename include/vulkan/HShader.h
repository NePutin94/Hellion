//
// Created by NePutin on 3/20/2023.
//

#ifndef HELLION_HSHADER_H
#define HELLION_HSHADER_H

#include <vulkan/vulkan.hpp>
#include <fstream>

namespace Hellion
{
    class HShader
    {
    private:
        std::string path;
        vk::UniqueShaderModule shaderModule;

        std::vector<char> read()
        {
            std::ifstream file(path, std::ios::ate | std::ios::binary);

            if(!file.is_open())
                throw std::runtime_error("failed to open file!");


            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            file.close();

            return buffer;
        }

    public:
        HShader(const std::string& filePath) : path(filePath)
        {}

        vk::UniqueShaderModule createShaderModule(vk::Device& device)
        {
            auto code = read();
            try
            {
                shaderModule = device.createShaderModuleUnique({vk::ShaderModuleCreateFlags(), code.size(), reinterpret_cast<const uint32_t*>(code.data())});
            } catch (vk::SystemError err)
            {
                throw std::runtime_error("failed to create shader module!");
            }
        }

        vk::UniqueShaderModule& getShaderModule()
        {
            return shaderModule;
        }
    };
}

#endif //HELLION_HSHADER_H
