#include "Renderer/API/Vulkan/Components/VulkanGlobalState/VulkanGlobalState.h"
#include "Renderer/API/Vulkan/VulkanContext.h"

int WIDTH = 1024;
int DEPTH = 1024;
float SCALE = 200.0;
float MAX_AMPLITUDE = 4.0;
float AMPLITUDE = 0.867;
float BASE_0 = 5.4f;
float BASE_1 = 2.4f;
float LACUNARITY = 2.0f;
float PERSISTENCE = 0.52f;
float FREQUENCY = 0.108f;
int OCTAVES = 14;
bool SHOULD_ANIMATE = true;
float MAX_ANIMATION_SPEED = 100.0f;
float ANIMATION_SPEED = 100.0f;
glm::vec3 CLEAR_COLOR = glm::vec3(25 / 255, 121 / 255, 155 / 255);

void new_landscape(VulkanContext context_vk, bool rebuild_indices = false) {
    // auto start = std::chrono::steady_clock::now();
    backend_vk::Pipeline compute_pipeline_data = {};
    for (auto pipeline : context_vk.m_pipelines) {
        if (pipeline.type == backend_vk::COMPUTE) {
            compute_pipeline_data = pipeline;
        }
    }
    context_vk.run_compute_shader(
        compute_pipeline_data.pipeline, compute_pipeline_data.pipeline_layout,
        context_vk.m_vulkan_global_state->m_compute_ds, false, rebuild_indices);
}

void update(VulkanContext context_vk, float delta_time) {
    context_vk.m_frame_time += delta_time;

    context_vk.m_cameraController->HandleInputOrbit(delta_time);

    memcpy(context_vk.m_vulkan_global_state->m_lightUBOMapped
               [context_vk.m_vulkan_global_state->m_currentFrame],
           &context_vk.m_vulkan_global_state->m_lights,
           sizeof(context_vk.m_vulkan_global_state->m_lights));

    context_vk.m_cameraController->UpdateCameraPosition(
        context_vk.m_vulkan_global_state->m_currentFrame, delta_time);

    if (!context_vk.m_vulkan_global_state->m_meshList.empty()) {
        int count = 0;
        for (auto& mesh : context_vk.m_vulkan_global_state->m_meshList) {
            if (count > 0) break;
            ModelUBO modelUBO{};
            modelUBO.model = glm::mat4(1.0f);
            modelUBO.model =
                glm::translate(modelUBO.model, glm::vec3(0.0f, 0.0f, 0.0f));
            modelUBO.model = glm::rotate(modelUBO.model, -glm::radians(90.0f),
                                         glm::vec3(0.0f, 1.0f, 0.0f));
            modelUBO.model =
                glm::scale(modelUBO.model, glm::vec3(0.5f, 0.5f, 0.5f));
            // glm::scale(modelUBO.model, glm::vec3(00.1f, 00.1f, 00.1f));

            memcpy(
                mesh.m_uniform_buffers_mapped[context_vk.m_vulkan_global_state
                                                  ->m_currentFrame],
                &modelUBO, sizeof(modelUBO));
            count++;
        }
    }
    if (SHOULD_ANIMATE) {
        context_vk.m_landscape_animation_time += delta_time;
        new_landscape(context_vk);
    }
}
