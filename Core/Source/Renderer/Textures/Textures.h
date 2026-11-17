#ifndef TEXTURES_H
#define TEXTURES_H

struct TextureResource {
    int tex_width;
    int tex_height;
    int tex_channels;

    unsigned char* pixels_sdr;
    float* pixels_hdr;
};

#endif  // !TEXTURES_H
