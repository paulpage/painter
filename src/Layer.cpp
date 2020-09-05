#include <QImage>

#include "Layer.h"

Layer::Layer(int width, int height, QString name): width(width), height(height), name(name)
{
    bitmap = bitmap_create(width, height);
    updateTexture();
    /* QImage image(bitmap.data, bitmap.width, bitmap.height, bitmap.width * 4, QImage::Format_RGBA8888, nullptr, nullptr); */
    /* texture = new QOpenGLTexture(image); */
}

Layer::Layer(QImage image, QString name): name(name)
{
    /* texture = new QOpenGLTexture(image); */
    width = image.width();
    height = image.height();
    bitmap = bitmap_create(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
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
    updateTexture();
}

Layer::~Layer()
{
}

void Layer::updateTexture()
{
    glEnable(GL_TEXTURE_2D);

    glDeleteTextures(1, &textureId); // This is safe to do because glDeleteTextures ignores 0
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TODO what does this do?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // TODO what does this do?
    QImage image(bitmap.data, bitmap.width, bitmap.height, bitmap.width * 4, QImage::Format_RGBA8888, nullptr, nullptr);
    /* QImage im(filename); */
    /* QImage tex = QGLWidget::convertToGLFormat(image); */

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glDisable(GL_TEXTURE_2D);

    /* return tex; */


    /* QImage image(bitmap.data, bitmap.width, bitmap.height, bitmap.width * 4, QImage::Format_RGBA8888, nullptr, nullptr); */
    /* glGenTextures( */
    /* /1* if (texture->isCreated()) { *1/ */
    /* /1*     texture->destroy(); *1/ */
    /*     texture->setData(image); */
    /*     /1* texture->create(); *1/ */
    /*     texture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest); */
    /* /1* } *1/ */
}
