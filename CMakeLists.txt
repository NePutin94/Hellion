cmake_minimum_required(VERSION 3.24)
project(Hellion)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG "3.3.8"
)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG "0.9.9.8"
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(nlohmannjson
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG "v3.11.2")
set(JSON_BuildTests OFF)
FetchContent_MakeAvailable(nlohmannjson)

set(FMT_DOC OFF)
set(FMT_TEST OFF)
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
        range-v3
        GIT_REPOSITORY https://github.com/ericniebler/range-v3
        GIT_TAG "0.12.0"
)
FetchContent_MakeAvailable(range-v3)

find_package(Vulkan REQUIRED)

FetchContent_Declare(
        VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)


FetchContent_Declare(
        tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
        GIT_TAG origin/release
)
FetchContent_MakeAvailable(tinyobjloader)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.89.4
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG "v0.9"
)
set(TRACY_NO_VSYNC_CAPTURE ON)
FetchContent_MakeAvailable(tracy)

file(GLOB CORE_SRC RELATIVE "${CMAKE_SOURCE_DIR}/" "src/*.cpp")
file(GLOB VULKAN_WRAPPER RELATIVE "${CMAKE_SOURCE_DIR}/" "src/vulkan/*.cpp")
file(GLOB IMGUI_SRC RELATIVE "${CMAKE_SOURCE_DIR}/" "${imgui_SOURCE_DIR}/*.cpp")
set(IMGUI_VULKAN_BACKEND ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
add_subdirectory(lib/stb)

add_executable(Hellion main.cpp ${CORE_SRC} ${VULKAN_WRAPPER} ${IMGUI_SRC} ${IMGUI_VULKAN_BACKEND})
message(STATUS ${Vulkan_FOUND})
target_link_libraries(Hellion PUBLIC nlohmann_json::nlohmann_json glm::glm glfw Vulkan::Vulkan range-v3::range-v3 fmt::fmt VulkanMemoryAllocator STB tinyobjloader TracyClient)
target_include_directories(Hellion PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)