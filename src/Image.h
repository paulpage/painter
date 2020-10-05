#include "Bitmap.h"

struct Layer {
    char *name;
    Bitmap *bitmap;
    int x;
    int y;
}

struct Image {
    int width;
    int height;
    char *filename;
    Layer *layers;
}

Image image_create(int width, int height, char *file_name);
void image_free(Image image);
int image_add_layer(int width, int height, int x, int y);
bool image_remove_layer(int id);
