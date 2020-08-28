#include "Bitmap.h"

Bitmap bitmap_create(int width, int height) {
    int size = width * height * 4;
    unsigned char *data = (unsigned char*) malloc(sizeof(unsigned char) * size);
    for (int i = 0; i < size; i++) {
        data[i] = 0;
    }
    return Bitmap {
        data,
        width,
        height,
        size,
    };
}

void bitmap_free(Bitmap *bitmap) {
    free(bitmap->data);
}

bool bitmap_set_pixel(Bitmap *bitmap, int x, int y, Color c)
{
    int w = bitmap->width;
    int h = bitmap->height;
    if (x >= 0 && x < w && y >= 0 && y < h)  {
        bitmap->data[(y * w + x) * 4] = c.r;
        bitmap->data[(y * w + x) * 4 + 1] = c.g;
        bitmap->data[(y * w + x) * 4 + 2] = c.b;
        bitmap->data[(y * w + x) * 4 + 3] = c.a;
        return true;
    }
    return false;
}
