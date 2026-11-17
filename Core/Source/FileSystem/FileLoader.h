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
                load_texture_file(albedoTextureFilePath);
        if (!metallicTextureFilePath.empty())
            loadedObject.textureResourceMap[METALLIC] =
                load_texture_file(metallicTextureFilePath);
        if (!roughnessTextureFilePath.empty())
            loadedObject.textureResourceMap[ROUGHNESS] =
                load_texture_file(roughnessTextureFilePath);
        if (!aoTextureFilePath.empty())
            loadedObject.textureResourceMap[AO] =
                load_texture_file(aoTextureFilePath);
        if (!normalTextureFilePath.empty())
            loadedObject.textureResourceMap[NORMAL] =
                load_texture_file(normalTextureFilePath);
        if (!emissiveTextureFilePath.empty())
            loadedObject.textureResourceMap[EMISSIVE] =
                load_texture_file(emissiveTextureFilePath);
        if (!ormTextureFilePath.empty())
            loadedObject.textureResourceMap[ORM] =
                load_texture_file(ormTextureFilePath);
        return loadedObject;
    }

    static TextureResource load_texture_file(std::string file_path) {
        int tex_width, tex_height, tex_channels;

        stbi_uc* pixels = stbi_load(file_path.c_str(), &tex_width, &tex_height,
                                    &tex_channels, STBI_rgb_alpha);

        TextureResource props;
        props.tex_width = tex_width;
        props.tex_height = tex_height;
        props.tex_channels = tex_channels;
        props.pixels_sdr = pixels;

        return props;
    }

    static TextureResource load_cubemap_texture(std::string file_path) {
        int tex_width, tex_height, tex_channels;

        float* pixels = stbi_loadf(file_path.c_str(), &tex_width, &tex_height,
                                   &tex_channels, STBI_rgb_alpha);

        TextureResource props;
        props.tex_width = tex_width;
        props.tex_height = tex_height;
        props.tex_channels = tex_channels;
        props.pixels_hdr = pixels;

        return props;
    }

   private:
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
