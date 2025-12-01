#version 450

layout(binding = 1) uniform sampler2D texSampler;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

layout(set = 0, binding = 2) uniform LightUBO {
    Light lights[16];
    int numLights;
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
