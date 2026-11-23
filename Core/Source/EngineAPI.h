#pragma once
#include <stdint.h>

/*
 * The EngineAPI is the boundary between the game dll and the engine static lib
 */

#define KB(x) ((size_t)(x) << 10)  // 1024 bytes
#define MB(x) ((size_t)(x) << 20)  // 1024 * 1024 bytes
#define GB(x) ((size_t)(x) << 30)  // 1024 * 1024 * 1024 bytes

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
        x = _x;
        y = _y;
        z = _z;
    }
};
struct GenSphereOptions {};

struct GenBoxOptions {
    Vec3f dimensions = Vec3f();
    bool draw_order_clockwise = false;
};

struct GenConeOptions {};

struct GenCylinderOptions {};

struct GenLandscapeOptions {};

struct EngineAPI {
    void (*set_clear_color)(float r, float g, float b);
    void (*gen_box_internal)(GenBoxOptions gen_options);

    inline void gen_box(GenBoxOptions gen_options = {}) { gen_box_internal(gen_options); }
};
