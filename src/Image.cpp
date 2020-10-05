#include <cstdlib>

#define STB_DS_IMPLEMENTATION
#include "lib/stb_ds.h"

#include "Image.h"

Image image_create(int width, int height, char *filename) {
    size_t len = strlen(filename);
    char *my_filename = malloc(len + 1);
    strncpy(my_filename, filename, len);

    Layer *layers = (*Layer)malloc(sizeof(Layer) * 8);
    return Image {
        width,
        height,
        my_filename,
        layers,
        0,
    };
}

void image_free(Image *image) {
    free(image->filename);
    arrfree(image->layers);
}

int image_add_layer(Image *image, int width, int height, int x, int y) {
    Bitmap bitmap = bitmap_create(width, height);
    Layer layer = {
        name,
        bitmap,
        x,
        y,
    };
    arrput(image->layers, layer);
    return arrlen(image->layers) - 1;
}

bool image_remove_layer(Image *image, int id) {
    arrdel(image->layers, id);
}
