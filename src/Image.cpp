#include <cstdlib>
#include <cstdio>

#define STB_DS_IMPLEMENTATION
#include "lib/stb_ds.h"

#include "Image.h"

Layer layer_create(const char *name, int x, int y, int width, int height) {
    char *my_name = (char*)malloc(strlen(name) + 1);
    strcpy(my_name, name);

    Bitmap bitmap = bitmap_create(width, height);

    Layer layer = { my_name, bitmap, x, y };
    return layer;
}

Layer layer_copy(Layer *original) {
    char *name = (char*)malloc(strlen(original->name) + 1);
    strcpy(name, original->name);
    Bitmap bitmap = bitmap_create(original->bitmap.width, original->bitmap.height);
    for (int i = 0; i < original->bitmap.size; i++) {
        bitmap.data[i] = original->bitmap.data[i];
    }
    Layer layer = { name, bitmap, original->x, original->y };
    return layer;
}

void layer_free(Layer *layer) {
    bitmap_free(&layer->bitmap);
}

Layer layer_create_from_bitmap(const char *name, int x, int y, Bitmap bitmap) {
    char *my_name = (char*)malloc(strlen(name) + 1);
    strcpy(my_name, name);

    Layer layer = { my_name, bitmap, x, y };
    return layer;
}

Image image_create(int width, int height, const char *filename) {
    char *my_filename = (char*)malloc(strlen(filename) + 1);
    strcpy(my_filename, filename);

    return Image {
        width,
        height,
        my_filename,
        0,
    };
}

Image image_copy(Image *original) {
    char *filename = (char*)malloc(strlen(original->filename) + 1);
    strcpy(filename, original->filename);
    Layer *layers = 0;
    for (int i = 0; i < arrlen(original->layers); i++) {
        arrput(layers, layer_copy(&original->layers[i]));
    }
    Image image = Image {
        original->width,
        original->height,
        filename,
        layers,
    };
    return image;
}

void image_free(Image image) {
    free(image.filename);
    for (int i = 0; i < arrlen(image.layers); i++) {
        layer_free(&image.layers[i]);
    }
    arrfree(image.layers);
}

void image_add_layer(Image *image, Layer layer) {
    arrput(image->layers, layer);
}

void image_remove_layer(Image *image, int id) {
    arrdel(image->layers, id);
}

void image_take_snapshot(Image *image, ImageHistory *hist) {

    for (int i = arrlen(hist->snapshots) - 1; i > hist->idx; i--) {
        arrpop(hist->snapshots);
    }
    arrput(hist->snapshots, image_copy(image));
    hist->idx++;
}

void image_undo(Image *image, ImageHistory *hist) {
    if (hist->idx > 0) {
        hist->idx--;
        image_free(*image);
        *image = image_copy(&hist->snapshots[hist->idx]);
    }
}

void image_redo(Image *image, ImageHistory *hist) {
    if (hist->idx < arrlen(hist->snapshots) - 1) {
        hist->idx++;
        image_free(*image);
        *image = image_copy(&hist->snapshots[hist->idx]);
    }
}
