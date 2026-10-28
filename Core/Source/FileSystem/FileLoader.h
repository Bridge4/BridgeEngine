#ifndef FILELOADER_H
#define FILELOADER_H
#include <iostream>
#include <stdexcept>
#include <string>

#include "Source/Renderer/RendererTypes.h"

class FileLoader {
   public:
    static LoadedObject LoadObject(UnloadedObject objectToLoad) {
        LoadedObject loadedObject;
        loadedObject.objProperties = LoadObjFile(objectToLoad.objFilePath);
        std::string albedoTextureFilePath =
            objectToLoad.materialTexFilePaths[ALBEDO];
        std::string metallicTextureFilePath =
            objectToLoad.materialTexFilePaths[METALLIC];
        std::string roughnessTextureFilePath =
            objectToLoad.materialTexFilePaths[ROUGHNESS];
        std::string aoTextureFilePath = objectToLoad.materialTexFilePaths[AO];
        std::string normalTextureFilePath =
            objectToLoad.materialTexFilePaths[NORMAL];
        std::string emissiveTextureFilePath =
            objectToLoad.materialTexFilePaths[EMISSIVE];

        if (!albedoTextureFilePath.empty())
            loadedObject.textureProperties[ALBEDO] =
                LoadTextureFile(albedoTextureFilePath);
        if (!metallicTextureFilePath.empty())
            loadedObject.textureProperties[METALLIC] =
                LoadTextureFile(metallicTextureFilePath);
        if (!roughnessTextureFilePath.empty())
            loadedObject.textureProperties[ROUGHNESS] =
                LoadTextureFile(roughnessTextureFilePath);
        if (!aoTextureFilePath.empty())
            loadedObject.textureProperties[AO] =
                LoadTextureFile(aoTextureFilePath);
        if (!normalTextureFilePath.empty())
            loadedObject.textureProperties[NORMAL] =
                LoadTextureFile(normalTextureFilePath);
        if (!emissiveTextureFilePath.empty())
            loadedObject.textureProperties[EMISSIVE] =
                LoadTextureFile(emissiveTextureFilePath);
        return loadedObject;
    }

   private:
    static TextureProperties LoadTextureFile(std::string filePath) {
        int texWidth, texHeight, texChannels;

        stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight,
                                    &texChannels, STBI_rgb_alpha);

        TextureProperties props;
        props.texWidth = texWidth;
        props.texHeight = texHeight;
        props.texChannels = texChannels;
        props.pixels = pixels;

        return props;
    }
    static ObjProperties LoadObjFile(std::string filePath) {
        tinyobj::attrib_t attrib;

        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              filePath.c_str())) {
            throw std::runtime_error(warn + err);
        }
        ObjProperties props;
        props.attrib = attrib;
        props.materials = materials;
        props.shapes = shapes;

        return props;
    }
};
#endif
