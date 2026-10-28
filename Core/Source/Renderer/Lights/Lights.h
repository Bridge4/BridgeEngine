#ifndef LIGHTS_H
#define LIGHTS_H
#include "Source/Math/Math.h"

struct Light {
    alignas(16) glm::vec4 position;   // xyz used, w ignored
    alignas(16) glm::vec4 color;      // rgb used, w ignored
    alignas(16) glm::vec4 intensity;  // x for intensity
    alignas(16) glm::vec4 lightDir;
};

struct LightUBO {
    alignas(16) Light lights[16];
    alignas(16) glm::ivec4 numLights;  // x for numLights
};

#endif
