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

bool bitmap_draw_pixel(Bitmap *bitmap, int x, int y, Color c)
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

void bitmap_draw_line(Bitmap *bitmap, int x1, int y1, int x2, int y2, Color c)
{
    bitmap_draw_pixel(bitmap, x1, y1, c);
    bitmap_draw_pixel(bitmap, x2, y2, c);
    int width = abs(x2 - x1);
    int height = abs(y2 - y1);
    int step = width > height ? width : height;
    if (step != 0) {
        double dx = ((double)x2 - (double)x1) / (double)step;
        double dy = ((double)y2 - (double)y1) / (double)step;
        for (int i = 0; i < step; i++) {
            bitmap_draw_pixel(
                    bitmap,
                    (int)((double)x1 + dx * (double)i),
                    (int)((double)y1 + dy * (double)i),
                    c);
        }
    }
}
