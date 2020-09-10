#ifndef BITMAP_H
#define BITMAP_H

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct Bitmap {
    unsigned char *data;
    int width;
    int height;
    int size;
};

Bitmap bitmap_create(int width, int height);
void bitmap_free(Bitmap *bitmap);
bool bitmap_get_pixel(Bitmap *bitmap, int x, int y, Color *color);
bool bitmap_blend_pixel(Bitmap *bitmap, int x, int y, Color color);
bool bitmap_blend(Bitmap *bitmap, Bitmap *other);
bool bitmap_draw_pixel(Bitmap *bitmap, int x, int y, Color color);
void bitmap_draw_line(Bitmap *bitmap, int x1, int y1, int x2, int y2, Color color);
void bitmap_fill(Bitmap *bitmap, int x, int y, Color color);

bool color_eq(Color c1, Color c2);

#endif // BITMAP_H
