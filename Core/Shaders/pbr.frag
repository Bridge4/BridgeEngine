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

layout(set = 1, binding = 1) uniform sampler2D albedoMap;
layout(set = 1, binding = 2) uniform sampler2D metallicMap;
layout(set = 1, binding = 3) uniform sampler2D roughnessMap;
layout(set = 1, binding = 4) uniform sampler2D aoMap;
layout(set = 1, binding = 5) uniform sampler2D normalMap;

layout(location = 0) in vec3 fragPos;     // world-space position
layout(location = 1) in vec3 fragNormal;  // world-space normal
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5);
}


const float PI = 3.14159265359;
float DistributionGGX(vec3 N, vec3 H, float roughness){
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float numerator = a2;
    float denom = (NdotH2 * (a2 -1.0) + 1);

    denom = PI * denom * denom;

    return numerator/denom;
}

float GeometrySchlickGGX(float NdotV, float roughness){
    float a1 = roughness + 1.0;
    float k = (a1*a1)/8.0;

    return NdotV/( NdotV * (1.0-k) + k );
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float geometryObstruction = GeometrySchlickGGX(NdotV, roughness);
    float geometryShadow = GeometrySchlickGGX(NdotL, roughness);

    return geometryObstruction * geometryShadow;
}


void main() {
    vec3 albedo     = pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2));
    vec3 normal     = texture(normalMap, fragTexCoord).rgb;
    float metallic  = texture(metallicMap, fragTexCoord).r;
    float roughness = texture(roughnessMap, fragTexCoord).r;
    float ao        = texture(aoMap, fragTexCoord).r;
    // Normalize inputs
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(camera.cameraPos.xyz - fragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightData.numLights.x; ++i){
        vec3 L = normalize(lightData.lights[i].position.xyz - fragPos);
        vec3 H = normalize(V + L);

        float distance = length(lightData.lights[i].position.xyz - fragPos);
        float attenuation = 1.0 / (distance * distance );

        vec3 radiance = lightData.lights[i].color.rgb * attenuation * lightData.lights[i].intensity.x;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, N, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N,L), 0.0) + 0.0001;
        vec3 specular = numerator/denom;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic;

        const float PI = 3.14159265359;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = ambient + Lo;


    outColor = vec4(color, 1.0);
}
