#include <cstdlib>

#define STB_DS_IMPLEMENTATION
#include "lib/stb_ds.h"

#include "Image.h"

Layer layer_create(const char *name, int x, int y, int width, int height) {
    size_t len = strlen(name);
    char *my_name = (char*)malloc(len + 1);
    strcpy(my_name, name);

    Bitmap bitmap = bitmap_create(width, height);

    Layer layer = { my_name, bitmap, x, y };
    return layer;
}


Layer layer_create_from_bitmap(const char *name, int x, int y, Bitmap bitmap) {
    size_t len = strlen(name);
    char *my_name = (char*)malloc(len + 1);
    strcpy(my_name, name);

    Layer layer = { my_name, bitmap, x, y };
    return layer;
}

Image image_create(int width, int height, const char *filename) {
    size_t len = strlen(filename);
    char *my_filename = (char*)malloc(len + 1);
    strcpy(my_filename, filename);

    return Image {
        width,
        height,
        my_filename,
        0,
    };
}

void image_free(Image *image) {
    free(image->filename);
    arrfree(image->layers);
}

void image_add_layer(Image *image, Layer layer) {
    arrput(image->layers, layer);
}

void image_remove_layer(Image *image, int id) {
    arrdel(image->layers, id);
}
