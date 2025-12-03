#version 450


struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};
layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} camera;

layout(set = 0, binding = 1) uniform LightUBO {
    Light lights[16];
    int numLights;
} lightData;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    //outColor = texture(texSampler, fragTexCoord);
    outColor = vec4(1,0,0,1);
}
