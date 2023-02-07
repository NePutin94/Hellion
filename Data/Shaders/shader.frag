#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float time;

layout(location = 0) out vec4 outColor;

void main() {
    float vWave = time;
    float r = texture(texSampler, fragTexCoord).r;
    float g = texture(texSampler, fragTexCoord ).g;
    float b = texture(texSampler, fragTexCoord + vWave).b;
    vec3 texture2d = vec3(r, g, b);
   // outColor = vec4(mix(fragColor * texture(texSampler, fragTexCoord + vWave).rgb,texture2d, vWave * 0.5), 1.0);
    outColor = vec4(fragColor * texture(texSampler, fragTexCoord + vWave).rgb, 1.0);
}
