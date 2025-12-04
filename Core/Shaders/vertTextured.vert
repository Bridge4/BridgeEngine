#version 450

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
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
    vec4 worldPos = modelData.model * vec4(inPosition, 1.0);
    //mat4 mvp = camera.proj * camera.view * worldPos;
    //mat4 mvp = camera.proj * camera.view * mat4(1);
    //gl_Position =  vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;
    fragNormal = normalize(mat3(modelData.model) * inNormal);
    fragTexCoord = inTexCoord;

    gl_Position = camera.proj * camera.view * worldPos;
}
