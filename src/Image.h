#include "Bitmap.h"

struct Layer {
    char *name;
    Bitmap bitmap;
    int x;
    int y;
};

struct Image {
    int width;
    int height;
    char *filename;
    Layer *layers;
};

Layer layer_create(const char *name, int x, int y, int width, int height);
Layer layer_create_from_bitmap(const char *name, int x, int y, Bitmap bitmap);

Image image_create(int width, int height, const char *file_name);
void image_free(Image image);
void image_add_layer(Image *image, Layer layer);
void image_remove_layer(int id);
