#include "Engine.h"

#include <imgui.h>

#include "Renderer/API/Vulkan/VulkanContext.h"
#include "Source/Renderer/Mesh/Mesh3D.h"
#include "Source/Renderer/Renderer.h"
#include "Source/Renderer/RendererTypes.h"
#include "Window/WindowingBackend.h"

void Engine::StartUp(ERenderingMode renderingMode,
                     WindowingBackend* windowingBackend) {
    m_renderer = new Renderer();
    m_renderer->StartUp(renderingMode, windowingBackend);
}

void Engine::Tick(float deltaTime) {
    m_renderer->Tick(m_loadedObjects, deltaTime);
}

void Engine::Destroy() { m_renderer->Destroy(); }

void Engine::set_clear_color(float r, float g, float b) {
    m_renderer->set_clear_color(r, g, b);
}

void Engine::generate_landscape() {
    Renderable mesh;
    m_renderer->generate_landscape();
}

void Engine::gen_box(GenBoxOptions gen_options) {
    float x_extent = gen_options.dimensions.x;
    float y_extent = gen_options.dimensions.y;
    float z_extent = gen_options.dimensions.z;
    // 8 corners of an axis-aligned box
    const glm::vec3 vertices[8] = {
        glm::vec3(0.0, 0.0, 0.0),                        // 0: min
        glm::vec3(x_extent * 1.0, 0.0, 0.0),             // 1: +X
        glm::vec3(x_extent * 1.0, y_extent * 1.0, 0.0),  // 2: +X +Y
        glm::vec3(0.0, y_extent * 1.0, 0.0),             // 3: +Y
        glm::vec3(0.0, 0.0, z_extent * 1.0),             // 4: +Z
        glm::vec3(x_extent * 1.0, 0.0, z_extent * 1.0),  // 5: +X +Z
        glm::vec3(x_extent * 1.0, y_extent * 1.0,
                  z_extent * 1.0),                      // 6: +X +Y +Z
        glm::vec3(0.0, y_extent * 1.0, z_extent * 1.0)  // 7: +Y +Z
    };

    // 36 indices (counter-clockwise winding for outward-facing triangles)
    const int indices_counter_clockwise[36] = {4, 5, 6, 4, 6, 7,
                                               // Back (-Z)
                                               1, 0, 3, 1, 3, 2,
                                               // Left (-X)
                                               0, 4, 7, 0, 7, 3,
                                               // Right (+X)
                                               5, 1, 2, 5, 2, 6,
                                               // Top (+Y)
                                               7, 6, 2, 7, 2, 3,
                                               // Bottom (-Y)
                                               0, 1, 5, 0, 5, 4

    };

    const int indices_clockwise[] = {// Back face (-Z)
                                     1, 0, 3, 3, 2, 1,
                                     // Front face (+Z)
                                     4, 7, 6, 6, 5, 4,
                                     // Left face (-X)
                                     0, 4, 7, 7, 3, 0,
                                     // Right face (+X)
                                     5, 1, 2, 2, 6, 5,
                                     // Bottom face (-Y)
                                     0, 1, 5, 5, 4, 0,
                                     // Top face (+Y)
                                     7, 6, 2, 2, 3, 7};
    if (gen_options.draw_order_clockwise) {
        // m_renderer->load_renderable(vertices, indices_clockwise);
    } else {
        // m_renderer->load_renderable(vertices, indices_counter_clockwise);
    }
    // Now we need to add this set of vertices and indices to the renderer's
    // buffers
}
namespace bridge_engine {

// This function generates a box with the specified options and adds it to the
// renderer's draw list

}  // namespace bridge_engine
