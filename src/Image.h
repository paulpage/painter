#ifndef IMAGE_H
#define IMAGE_H

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

struct ImageHistory {
    Image *snapshots;
    int idx;
};

Layer layer_create(const char *name, int x, int y, int width, int height);
Layer layer_create_from_bitmap(const char *name, int x, int y, Bitmap bitmap);
Layer layer_copy(Layer *original);
void layer_free(Layer *layer);

Image image_create(int width, int height, const char *file_name);
void image_free(Image image);
Image image_copy(Image *original);
void image_add_layer(Image *image, Layer layer);
void image_remove_layer(int id);
void image_take_snapshot(Image *image, ImageHistory *hist);
void image_undo(Image *image, ImageHistory *hist);
void image_redo(Image *image, ImageHistory *hist);

#endif // IMAGE_H
