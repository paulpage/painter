#include <cstdlib>

#include "Image.h"

Image image_create(int width, int height, char *file_name) {
    Layer *layers = (*Layer)malloc(sizeof(Layer) * 8);
    return Image {
        width,
        height,
        file_name,
        layers,
        0,
    };
}

void image_free(Image image) {

}

int image_add_layer(int width, int height, int x, int y) {
   
}

bool image_remove_layer(int id) {
    if (id < num_layers) {

    }
}
