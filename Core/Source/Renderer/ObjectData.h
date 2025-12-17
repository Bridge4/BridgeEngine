#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <string>
#include <unordered_map>

#include "Source/Renderer/API/Vulkan/Components/Mesh/Mesh3D.h"

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
