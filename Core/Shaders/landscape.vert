#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform sampler2D heightMap;
uniform float heightScale;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    TexCoord = aTexCoord;
    
    // Sample the height value from the red channel (or green/blue)
    float height = texture(heightMap, TexCoord).r;
    
    // Modify vertex position along the Y (or Z) axis
    vec3 displacedPos = aPos;
    displacedPos.y += height * heightScale;
    
    gl_Position = projection * view * model * vec4(displacedPos, 1.0);
}
