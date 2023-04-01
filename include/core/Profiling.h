//
// Created by NePutin on 4/1/2023.
//

#ifndef HELLION_PROFILING_H
#define HELLION_PROFILING_H

#include <vulkan/vulkan.hpp>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

namespace Hellion
{
#ifdef HELLION_PROFILING
#define HELION_ZONE_PROFILING() ZoneScopedN(__FUNCSIG__);
#define HELION_GPUZONE_PROFILING(ctx, buf, name) TracyVkZone(ctx,buf,name)
#else
#define HELION_ZONE_PROFILING()
#define HELION_ZONE_PROFILING(name)
#endif

} // Hellion

#endif //HELLION_PROFILING_H
