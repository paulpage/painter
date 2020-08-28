#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QScrollArea>
#include <QList>
#include <QString>
#include <QScrollBar>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShader>
#include <QOpenGLBuffer>
#include <QImageReader>
#include <QMessageBox>
#include <QGuiApplication>
#include <QDir>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QExposeEvent>
#include <QColor>

#include "Bitmap.h"

class ImageWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    using QOpenGLWidget::QOpenGLWidget;

    QImage image;
    Bitmap bitmap;

    ImageWidget();
    ~ImageWidget();

    /* void setBitmap(Bitmap *bitmap); */
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    bool loadFile(QString fileName);
    void adjustSize();
    void setVertexData();
    void scaleImage(double factor);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int width, int height);
private:
    /* QLabel *imageLabel; */ 
    /* QOpenGLWidget *imageWidget; */
    double scaleFactor = 1;
    QPoint mousePosition;
    GLfloat offsetX = 0;
    GLfloat offsetY = 0;
    QOpenGLTexture *texture;
    bool isMiddleButtonDown = false;
    bool isLeftButtonDown = false;
    QOpenGLShaderProgram *program;
    void updateTexture();
    /* QOpenGLBuffer vbo; */
};

#endif
