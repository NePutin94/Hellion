//
// Created by NePutin on 3/24/2023.
//

#ifndef HELLION_HAPP_H
#define HELLION_HAPP_H

#include "vulkan/HPipeline.h"
#include "vulkan/HDescriptorSetLayout.h"
#include "vulkan/HBuffer.h"
#include "vulkan/HRenderer.h"
#include "vulkan/RenderSystem.h"

namespace Hellion
{

    class HApp
    {
    public:
        HApp()
        {
            createGraphicsPipeline();
        }

        ~HApp()
        {

        }

        void run()
        {}

        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

    private:
        void createGraphicsPipeline()
        {
        }

        void drawFrame()
        {
            while(true)
            {
                if(auto commandBuffer = renderer.beginFrame())
                {
                    int frameIndex = renderer.getFrameIndex();
                    renderer.beginSwapChainRenderPass(commandBuffer);


                    renderer.endSwapChainRenderPass(commandBuffer);
                    renderer.endFrame();
                }
            }
        }

        HWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
        HDevice device{window};
        HRenderer renderer{window, device};
        RenderSystem renderSystem{device, renderer.getSwapChainRenderPass(), *renderer.getSwapChain()};
        //HSwapChain swapChain{window, device};

        std::vector<HBuffer> uniformBuffers;
    };

} // Hellion

#endif //HELLION_HAPP_H
