#ifndef LAYER_H
#define LAYER_H

#include <QOpenGLTexture>

#include "Bitmap.h"

class Layer {
public:
    Layer(int width, int height, QString name = QString("Unnamed Layer"));
    Layer(QImage image, QString name = QString("Unnamed Layer"));
    ~Layer();

    QString name;
    QOpenGLTexture *texture;
    Bitmap bitmap;
};

#endif
