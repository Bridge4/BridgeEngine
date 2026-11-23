#pragma once
#ifdef _WIN32
#ifdef GAME_EXPORTS
#define GAME_API extern "C" __declspec(dllexport)
#else
#define GAME_API extern "C"
#endif
#else
#define GAME_API extern "C"
#endif

#include "EngineAPI.h"

GAME_API void game_init(EngineAPI* engine) { engine->set_clear_color(0.0, 0.0, 0.0); }

GAME_API void game_update(EngineAPI* engine) {
    // I want to make it so that the draw list is defined by the game
    // If I remove the line below, it should remove the landscape from the
    // draw list
    // engine->set_clear_color(1.0, 0.20, 0.0);
    // engine->gen_box();
    //  Landscape landscape = engine->gen_mesh_landscape(params);
    //  Mesh sphere = engine->gen_mesh_sphere(params);
    //
    //
    //
    //
    //  Mesh square = engine->gen_mesh_square(params);
    //  square.location = Vec3(1.0, 1.0, 300.0); // This will move the square
    //  square.rotation = Vec3(1.0, 1.0, 300.0); // This will rotate the square
    //
    //
    //
    //
    //  engine->gen_mesh_cone(params);
    //  engine->gen_mesh_cylinder(params);
    //  engine->load_mesh(file_path);
    //
    //
    //
    //
    //
    //
    //
    //  Verbose Mesh Load
    //  bool is_file_supported = engine->check_file_support(file_path);
    //  if (is_file_supported){
    //  ObjData obj_data = engine->parse_file_obj();
    //  GltfData gltf_data = engine->parse_file_gltf();
    //  UsdData usd_data = engine->parse_file_usd();
    //  Mesh mesh_obj = engine->load_mesh_from_obj_data(obj_data);
    //  if (gltf_data.type == MESH){
    //  Mesh mesh_gltf = engine->load_mesh_from_gltf_data(gltf_data);
    //  }
    //  else if (gltf_data.type == SCENE){
    //  Scene scene_gltf = engine->load_scene_from_gltf_data(gltf_data);
    //
    //  }
    //  Mesh mesh_usd = engine->load_mesh_from_usd_data(usd_data);
    //
    //
    //
}
