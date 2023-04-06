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
#include "vulkan/CanvasSystem.h"
#include "HCamera.h"
#include <tracy/Tracy.hpp>

namespace Hellion
{

    class HApp
    {
    public:
        HApp()
        {
            canvas.line({0.f, 0.f, 0.f}, {10.f, 10.f, 0.f}, {0, 0, 255, 255});
            canvas.plane3d({0,1.5,0}, {0,1,0}, {1,0,0}, 40, 40, 10.0f, 10.0f, {1,0,0,1}, {0,255,0,1});
            canvas.init(renderer.getSwapChainRenderPass(), *renderer.getSwapChain());
        }

        ~HApp()
        {
        }

        void run()
        {
            while(!window.shouldClose())
            {
                HELLION_ZONE_PROFILING()
                glfwPollEvents();
                renderer.getImGuiRender().NewFrame();

                camera.update(window.getWindow());

                if(auto commandBuffer = renderer.beginFrame())
                {
                    int frameIndex = renderer.getFrameIndex();

                    renderer.getImGuiRender().render();
                    renderer.beginSwapChainRenderPass(commandBuffer);

                    auto exte = renderer.getSwapChain()->getSwapChainExtent();
                    renderSystem.updateBuffers(renderer.getFrameIndex(), exte.width, exte.height, camera);
                    renderSystem.draw(commandBuffer, renderer.getFrameIndex(), renderer.getCurrentTracyCtx());

                    canvas.updateBuffers(renderer.getFrameIndex(), exte.width, exte.height, camera);
                    canvas.draw(commandBuffer, renderer.getFrameIndex(), renderer.getCurrentTracyCtx());

                    renderer.endSwapChainRenderPass(commandBuffer);
                    renderer.endFrame();
                }
            }
            device.getDevice().waitIdle();
        }

        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

    private:
        HCamera camera{{800, 600}};
        HWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
        HDevice device{window};
        HRenderer renderer{window, device};
        RenderSystem renderSystem{device, renderer.getSwapChainRenderPass(), *renderer.getSwapChain()};
        CanvasSystem canvas{device};
        //HSwapChain swapChain{window, device};

        std::vector<HBuffer> uniformBuffers;
    };

} // Hellion

#endif //HELLION_HAPP_H
