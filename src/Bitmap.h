#ifndef BITMAP_H
#define BITMAP_H

class Bitmap {
public:
    Bitmap(int width, int height);
    ~Bitmap();

    unsigned char *data;
    int width;
    int height;
    int size;
};

#endif // BITMAP_H
