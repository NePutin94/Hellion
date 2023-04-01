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
#include <tracy/Tracy.hpp>

namespace Hellion
{

    class HApp
    {
    public:
        HApp()
        {}

        ~HApp()
        {
        }

        void run()
        {
            while(!window.shouldClose())
            {
                ZoneScoped;
                glfwPollEvents();
                if(auto commandBuffer = renderer.beginFrame())
                {
                    int frameIndex = renderer.getFrameIndex();
                    renderer.beginSwapChainRenderPass(commandBuffer);

                    auto exte = renderer.getSwapChain()->getSwapChainExtent();
                    renderSystem.updateBuffers(renderer.getFrameIndex(), exte.width, exte.height);
                    renderSystem.draw(commandBuffer, renderer.getFrameIndex(), renderer.getCurrentTracyCtx());

                    renderer.endSwapChainRenderPass(commandBuffer);
                    renderer.endFrame();
                }
            }
        }

        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

    private:
        HWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
        HDevice device{window};
        HRenderer renderer{window, device};
        RenderSystem renderSystem{device, renderer.getSwapChainRenderPass(), *renderer.getSwapChain()};
        //HSwapChain swapChain{window, device};

        std::vector<HBuffer> uniformBuffers;
    };

} // Hellion

#endif //HELLION_HAPP_H
