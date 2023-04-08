#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

#define MAX_NUM_JOINTS 128

layout (set = 1, binding = 0) uniform UBONode {
    mat4 matrix;
    mat4 jointMatrix[MAX_NUM_JOINTS];
    float jointCount;
} node;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inJoint0;
layout (location = 5) in vec4 inWeight0;
layout (location = 6) in vec4 inColor0;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * node.matrix * vec4(inPos, 1.0);
    fragColor = inColor0.xyz;
    fragTexCoord = inUV0;
}