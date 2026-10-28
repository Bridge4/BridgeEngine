#ifndef CAMERA_H
#define CAMERA_H
#include "Source/Math/Math.h"
// Scene-Wide Data
struct CameraUBO {
    alignas(16) glm::mat4 view;       // 64 bytes
    alignas(16) glm::mat4 proj;       // 64 bytes
    alignas(16) glm::vec4 cameraPos;  // 16 bytes (use xyz, ignore w)
};

#endif  // CAMERA_H
