#include <stb_image.h>
#include <string>
#include <tiny_obj_loader.h>


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
    std::string textureFilePath;
};

struct LoadedObject {
    ObjProperties objProperties;
    TextureProperties textureProperties;
};
