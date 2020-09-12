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

    int x = 0;
    int y = 0;
    int width;
    int height;
    QString name;
    Bitmap bitmap;
    bool isVisible = true;
};

#endif
