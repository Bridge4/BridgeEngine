#ifndef TEXTURES_H
#define TEXTURES_H

struct TextureProperties {
    int texWidth, texHeight, texChannels;

    unsigned char* pixels;
};

#endif  // !TEXTURES_H
