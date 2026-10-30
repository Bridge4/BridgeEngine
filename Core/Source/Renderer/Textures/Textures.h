#ifndef TEXTURES_H
#define TEXTURES_H

struct TextureResource {
    int texWidth;
    int texHeight;
    int texChannels;

    unsigned char* pixels;
};

#endif  // !TEXTURES_H
