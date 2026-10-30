#ifndef FILELOADER_H
#define FILELOADER_H
#include <iostream>
#include <stdexcept>
#include <string>

#include "Source/Renderer/RendererTypes.h"

class FileLoader {
   public:
    static ObjResource LoadObject(ObjAsset objectToLoad) {
        ObjResource loadedObject;
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
        std::string ormTextureFilePath = objectToLoad.materialTexFilePaths[ORM];

        if (!albedoTextureFilePath.empty())
            loadedObject.textureResourceMap[ALBEDO] =
                LoadTextureFile(albedoTextureFilePath);
        if (!metallicTextureFilePath.empty())
            loadedObject.textureResourceMap[METALLIC] =
                LoadTextureFile(metallicTextureFilePath);
        if (!roughnessTextureFilePath.empty())
            loadedObject.textureResourceMap[ROUGHNESS] =
                LoadTextureFile(roughnessTextureFilePath);
        if (!aoTextureFilePath.empty())
            loadedObject.textureResourceMap[AO] =
                LoadTextureFile(aoTextureFilePath);
        if (!normalTextureFilePath.empty())
            loadedObject.textureResourceMap[NORMAL] =
                LoadTextureFile(normalTextureFilePath);
        if (!emissiveTextureFilePath.empty())
            loadedObject.textureResourceMap[EMISSIVE] =
                LoadTextureFile(emissiveTextureFilePath);
        if (!ormTextureFilePath.empty())
            loadedObject.textureResourceMap[ORM] =
                LoadTextureFile(ormTextureFilePath);
        return loadedObject;
    }

   private:
    static TextureResource LoadTextureFile(std::string filePath) {
        int texWidth, texHeight, texChannels;

        stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight,
                                    &texChannels, STBI_rgb_alpha);

        TextureResource props;
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
