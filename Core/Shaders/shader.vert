#version 450

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} camera;

layout(set = 1, binding = 0) uniform ModelUBO {
    mat4 model;
} modelData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    mat4 mvp = camera.proj * camera.view * modelData.model;
    gl_Position = mvp * vec4(inPosition, 1.0);
    fragPos = vec3(modelData.model * vec4(inPosition, 1.0));
    fragNormal = mat3(transpose(inverse(modelData.model))) * inNormal;
    fragTexCoord = inTexCoord;
}
