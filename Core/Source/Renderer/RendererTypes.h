#ifndef RENDERERTYPES_H
#define RENDERERTYPES_H
#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <string>
#include <unordered_map>

enum MaterialEnums { ALBEDO, METALLIC, ROUGHNESS, AO, NORMAL, EMISSIVE };
enum Mesh3DTypes { TEXTURED, UNTEXTURED };

struct ObjProperties {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
};

struct TextureProperties {
    int texWidth, texHeight, texChannels;

    unsigned char* pixels;
};

struct UnloadedObject {
    std::string objFilePath;
    std::unordered_map<MaterialEnums, std::string> materialTexFilePaths;
};

struct LoadedObject {
    ObjProperties objProperties;
    std::unordered_map<MaterialEnums, TextureProperties> textureProperties;
};
#endif
