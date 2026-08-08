#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb/stb_image.h"

int image_average_color(const char* path, ModelColor* color) {
    if (path == NULL || color == NULL)
        return 0;

    int width;
    int height;
    int channels;
    unsigned char* pixels = stbi_load(path, &width, &height, &channels, 3);
    if (pixels == NULL || width <= 0 || height <= 0) {
        stbi_image_free(pixels);
        return 0;
    }

    unsigned long long red = 0;
    unsigned long long green = 0;
    unsigned long long blue = 0;
    size_t count = (size_t)width * (size_t)height;
    for (size_t i = 0; i < count; ++i) {
        red += pixels[i * 3];
        green += pixels[i * 3 + 1];
        blue += pixels[i * 3 + 2];
    }

    stbi_image_free(pixels);
    *color = (ModelColor){red / (255. * count), green / (255. * count),
                          blue / (255. * count)};
    return 1;
}
