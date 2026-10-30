#ifndef RENDERERTYPES_H
#define RENDERERTYPES_H
#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <string>
#include <unordered_map>

#include "Materials/Materials.h"
#include "Textures/Textures.h"

enum Mesh3DTypes { TEXTURED, UNTEXTURED };

struct ObjProperties {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
};

struct ObjAsset {
    std::string objFilePath;
    std::unordered_map<MaterialEnums, std::string> materialTexFilePaths;
};

struct ObjResource {
    ObjProperties objProperties;
    std::unordered_map<MaterialEnums, TextureResource> textureResourceMap;
};

enum ERenderingMode { Standard, Bindless };
#endif
