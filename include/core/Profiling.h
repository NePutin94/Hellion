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
#ifdef __GNUC__
#define HELLION_ZONE_PROFILING() ZoneScopedN(__PRETTY_FUNCTION__);
#else
#define HELLION_ZONE_PROFILING() ZoneScopedN(__FUNCSIG__);
#endif
#define HELLION_GPUZONE_PROFILING(ctx, buf, name) TracyVkZone(ctx,buf,name)
#else
#define HELLION_ZONE_PROFILING()
#define HELLION_ZONE_PROFILING(name)
#define HELLION_GPUZONE_PROFILING(ctx, buf, name)
#endif

} // Hellion

#endif //HELLION_PROFILING_H
