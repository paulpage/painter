#include "Bitmap.h"
#include "common.h"

#include <cstdlib>
#include <cstdio>

struct Point {
    int x;
    int y;
};

struct PointQueue {
    int size;
    int capacity;
    int start;
    int end;
    Point *data;
};

PointQueue pq_create()
{
    return PointQueue {
        0,
        1024, // TODO make this higher when we know it works
        0,
        0,
        (Point*)malloc(sizeof(Point) * 1024),
    };
}

void pq_free(PointQueue *q)
{
    free(q->data);
}

void pq_push(PointQueue *q, Point p)
{
    if (q->end == q->capacity) {
        int oldCapacity = q->capacity;
        q->capacity *= 2;
        Point *newData = (Point*)malloc(sizeof(Point) * q->capacity);
        for (int i = 0; i < oldCapacity; i++) {
            newData[i] = q->data[i];
        }
        free(q->data);
        q->data = newData;
    }
    q->data[q->end] = p;
    q->size += 1;
    q->end += 1;
}

bool pq_pop(PointQueue *q, Point *p)
{
    if (q->size <= 0) {
        return false;
    }
    *p = q->data[q->start];
    q->start += 1;
    q->size -= 1;
    return true;
}

bool color_eq(Color c1, Color c2)
{
    return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a);
}

Bitmap bitmap_create(int width, int height)
{
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

Bitmap bitmap_create_rotated(Bitmap *old) {
    Bitmap bitmap = bitmap_create(old->height, old->width);
    for (int y = 0; y < old->height; y++) {
        for (int x = 0; x < old->width; x++) {
            int oldOffset = (y * old->width + x) * 4;
            int offset = (  (old->height - 1 - y) +  (x * old->height)  ) * 4;
            bitmap.data[offset] = old->data[oldOffset];
            bitmap.data[offset + 1] = old->data[oldOffset + 1];
            bitmap.data[offset + 2] = old->data[oldOffset + 2];
            bitmap.data[offset + 3] = old->data[oldOffset + 3];
        }
    }
    return bitmap;
}

void bitmap_free(Bitmap *bitmap)
{
    free(bitmap->data);
}

bool bitmap_get_pixel(Bitmap *bitmap, int x, int y, Color *color)
{
    int w = bitmap->width;
    int h = bitmap->height;
    if (x >= 0 && x < w && y >= 0 && y < h)  {
        color->r = bitmap->data[(y * w + x) * 4];
        color->g = bitmap->data[(y * w + x) * 4 + 1];
        color->b = bitmap->data[(y * w + x) * 4 + 2];
        color->a = bitmap->data[(y * w + x) * 4 + 3];
        return true;
    }
    return false;
}

bool bitmap_draw_pixel(Bitmap *bitmap, int x, int y, Color color)
{
    int offset = (y * bitmap->width + x) * 4;
    if (x >= 0 && x < bitmap->width && y >= 0 && y < bitmap->height)  {
        bitmap->data[offset] = color.r;
        bitmap->data[offset + 1] = color.g;
        bitmap->data[offset + 2] = color.b;
        bitmap->data[offset + 3] = color.a;
        return true;
    }
    return false;
}

bool bitmap_blend(Bitmap *bitmap, Bitmap *other, int offset_x, int offset_y)
{
    // Only blend the portion of the other bitmap that overlaps with the base
    int width = MIN(other->width, bitmap->width - offset_x);
    int height = MIN(other->height, bitmap->height - offset_y);
    if (bitmap->width >= other->width && bitmap->height >= other->height) {
        for (int y = offset_y; y < offset_y + height; y++) {
            for (int x = offset_x; x < offset_x + width; x++) {
                int i = (y * bitmap->width + x) * 4;
                int oi = ((y - offset_y) * width + (x - offset_x)) * 4;

                // The common cases are 0 or full alpha, so make those fast
                if (other->data[oi + 3] == 0) {
                    continue;
                }
                if (other->data[oi + 3] == 255) {
                    bitmap->data[i] = other->data[oi];
                    bitmap->data[i + 1] = other->data[oi + 1];
                    bitmap->data[i + 2] = other->data[oi + 2];
                    bitmap->data[i + 3] = other->data[oi + 3];
                    continue;
                }

                // Uncommon case (normal blending)
                double a1 = (double)other->data[oi + 3] / 255.0f;
                double a2 = (double)bitmap->data[i + 3] / 255.0f;
                double factor = a2 * (1.0f - a1);
                bitmap->data[i]     = (unsigned char)((double)bitmap->data[i]     * a1 + (double)other->data[oi]     * factor) / (a1 + factor);
                bitmap->data[i + 1] = (unsigned char)((double)bitmap->data[i + 1] * a1 + (double)other->data[oi + 1] * factor) / (a1 + factor);
                bitmap->data[i + 2] = (unsigned char)((double)bitmap->data[i + 2] * a1 + (double)other->data[oi + 2] * factor) / (a1 + factor);
                bitmap->data[i + 3] = (unsigned char)((a1 + factor) * 255.0f);
            }
        }
        return true;
    }
    return false;
}

void bitmap_draw_line(Bitmap *bitmap, int x1, int y1, int x2, int y2, Color color)
{
    bitmap_draw_pixel(bitmap, x1, y1, color);
    bitmap_draw_pixel(bitmap, x2, y2, color);
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
                    color);
        }
    }
}

void bitmap_fill(Bitmap *bitmap, int x, int y, Color color)
{
    Color targetColor;
    if (bitmap_get_pixel(bitmap, x, y, &targetColor)) {
        PointQueue q = pq_create();
        Color currentColor;
        Point p;
        if (bitmap_get_pixel(bitmap, x, y, &currentColor)) {
            if (color_eq(currentColor, color)) {
                pq_free(&q);
                return;
            }
        }
        bitmap_draw_pixel(bitmap, x, y, color);
        pq_push(&q, Point { x, y });
        while (q.size > 0) {
            if (pq_pop(&q, &p)) {
                if (bitmap_get_pixel(bitmap, p.x - 1, p.y, &currentColor)) {
                    if (color_eq(currentColor, targetColor)) {
                        bitmap_draw_pixel(bitmap, p.x - 1, p.y, color);
                        pq_push(&q, Point { p.x - 1, p.y });
                    }
                }
                if (bitmap_get_pixel(bitmap, p.x + 1, p.y, &currentColor)) {
                    if (color_eq(currentColor, targetColor)) {
                        bitmap_draw_pixel(bitmap, p.x + 1, p.y, color);
                        pq_push(&q, Point { p.x + 1, p.y });
                    }
                }
                if (bitmap_get_pixel(bitmap, p.x, p.y - 1, &currentColor)) {
                    if (color_eq(currentColor, targetColor)) {
                        bitmap_draw_pixel(bitmap, p.x, p.y - 1, color);
                        pq_push(&q, Point { p.x, p.y - 1 });
                    }
                }
                if (bitmap_get_pixel(bitmap, p.x, p.y + 1, &currentColor)) {
                    if (color_eq(currentColor, targetColor)) {
                        bitmap_draw_pixel(bitmap, p.x, p.y + 1, color);
                        pq_push(&q, Point { p.x, p.y + 1 });
                    }
                }
            }
        }
        pq_free(&q);
    }
}
