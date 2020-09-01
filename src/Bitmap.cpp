#include "Bitmap.h"

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
    int w = bitmap->width;
    int h = bitmap->height;
    if (x >= 0 && x < w && y >= 0 && y < h)  {
        bitmap->data[(y * w + x) * 4] = color.r;
        bitmap->data[(y * w + x) * 4 + 1] = color.g;
        bitmap->data[(y * w + x) * 4 + 2] = color.b;
        bitmap->data[(y * w + x) * 4 + 3] = color.a;
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
