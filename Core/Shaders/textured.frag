#version 450

struct Light {
    vec4 position;   // xyz = world-space position
    vec4 color;      // rgb = light color
    vec4 intensity; //  x = intensity
};

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec4 cameraPos; // xyz used
} camera;

layout(set = 0, binding = 1) uniform LightUBO {
    Light lights[16];
    ivec4 numLights;
} lightData;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;     // world-space position
layout(location = 1) in vec3 fragNormal;  // world-space normal
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    // Base albedo from texture
    vec3 albedo = texture(texSampler, fragTexCoord).rgb;

    // Normalize inputs
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(camera.cameraPos.xyz - fragPos);

    // Ambient term
    vec3 ambient = 0.05 * albedo;

    vec3 result = ambient;

    for (int i = 0; i < lightData.numLights.x; ++i) {
        vec3 Lpos = lightData.lights[i].position.xyz;
        vec3 Ldir = normalize(Lpos - fragPos);

        // Diffuse
        float diff = max(dot(N, Ldir), 0.0);

        // Blinn-Phong specular
        vec3 H = normalize(Ldir + V);
        float spec = pow(max(dot(N, H), 0.0), 64.0); // shininess = 64

        vec3 lightColor = lightData.lights[i].color.rgb * lightData.lights[i].intensity.x;

        vec3 diffuse  = diff * albedo * lightColor;
        vec3 specular = spec * lightColor;

        result += diffuse + specular;
    }

    outColor = vec4(result, 1.0);
}
