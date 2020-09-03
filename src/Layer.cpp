#include "Layer.h"

Layer::Layer(int width, int height, QString name): name(name)
{
    bitmap = bitmap_create(width, height);
    QImage image(bitmap.data, bitmap.width, bitmap.height, bitmap.width * 4, QImage::Format_RGBA8888, nullptr, nullptr);
    texture = new QOpenGLTexture(image);
}

Layer::Layer(QImage image, QString name): name(name)
{
    texture = new QOpenGLTexture(image);
    bitmap = bitmap_create(image.width(), image.height());
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            QRgb c = image.pixel(x, y);
            Color color = {
                (unsigned char)qRed(c),
                (unsigned char)qGreen(c),
                (unsigned char)qBlue(c),
                (unsigned char)qAlpha(c),
            };
            bitmap_draw_pixel(&bitmap, x, y, color);
        }
    }
}

Layer::~Layer()
{
}
