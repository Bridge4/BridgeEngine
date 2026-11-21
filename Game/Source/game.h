#pragma once
#ifdef GAME_EXPORTS
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif

#include "Renderer/API/Vulkan/VulkanContext.h"

extern "C" GAME_API void update(VulkanContext context_vk, float deltaTime);
extern "C" GAME_API void new_landscape(VulkanContext context_vk,
                                       bool rebuild_indices = false);
