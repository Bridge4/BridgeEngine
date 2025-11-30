//#include "FileLoader.h"
//#include <stdexcept>
//#include <vector>
//
//LoadedObject FileLoader::LoadObject(UnloadedObject objectToLoad) {
//    LoadedObject loadedObject;
//    loadedObject.objProperties = LoadObjFile(objectToLoad.objFilePath);
//    loadedObject.textureProperties = LoadTextureFile(objectToLoad.textureFilePath);
//    return loadedObject;
//}
//
//
//TextureProperties FileLoader::LoadTextureFile(std::string filePath) {
//    int texWidth, texHeight, texChannels;
//
//    stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//
//    TextureProperties props;
//    props.texWidth = texWidth;
//    props.texHeight = texHeight;
//    props.texChannels = texChannels;
//    props.pixels = pixels;
//
//    return props;
//}
//
//ObjProperties FileLoader::LoadObjFile(std::string filePath) {
//    tinyobj::attrib_t attrib;
//
//    std::vector<tinyobj::shape_t> shapes;
//    std::vector<tinyobj::material_t> materials;
//    std::string warn, err;
//
//    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
//        throw std::runtime_error(warn + err);
//    }
//    ObjProperties props;
//    props.attrib = attrib;
//    props.materials = materials;
//    props.shapes = shapes;
//
//    return props;
//}
