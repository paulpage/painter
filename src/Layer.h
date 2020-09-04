#ifndef LAYER_H
#define LAYER_H

#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "Bitmap.h"

class Layer
{
public:
    Layer(int width, int height, QString name = QString("Unnamed Layer"));
    Layer(QImage image, QString name = QString("Unnamed Layer"));
    ~Layer();
    void updateTexture();

    int width;
    int height;
    QString name;
    QOpenGLTexture *texture;
    Bitmap bitmap;
};

#endif
