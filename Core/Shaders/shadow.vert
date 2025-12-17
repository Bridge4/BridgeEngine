#version 450

layout(push_constant) uniform PushConstants {
    mat4 lightViewProj;
    mat4 model;           // per-object model matrix
} pc;

layout(location = 0) in vec3 inPosition;

void main() {
    gl_Position = pc.lightViewProj * pc.model * vec4(inPosition, 1.0);
}
