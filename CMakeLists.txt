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

file(GLOB CORE_SRC RELATIVE "${CMAKE_SOURCE_DIR}/" "src/*.cpp")
file(GLOB VULKAN_WRAPPER RELATIVE "${CMAKE_SOURCE_DIR}/" "src/vulkan/*.cpp")

add_executable(Hellion main.cpp ${CORE_SRC} ${VULKAN_WRAPPER})
message(STATUS ${Vulkan_FOUND})
target_link_libraries(Hellion PUBLIC nlohmann_json::nlohmann_json glm::glm glfw Vulkan::Vulkan range-v3::range-v3 fmt::fmt)
