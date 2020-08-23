#include <cstdlib>
#include "Bitmap.h"

Bitmap::Bitmap(int width, int height): width(width), height(height) {
/* Bitmap::Bitmap(int width, int height) { */
    // TODO remove!
    /* width = 100; */
    /* height = 100; */
    size = width * height * 4;
    data = (unsigned char*) malloc(sizeof(unsigned char) * size);
    for (int i = 0; i < size; i++) {
        data[i] = 0;
    }
}

Bitmap::~Bitmap() {
    free(data);
}
