#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QScrollArea>
#include <QString>
#include <QScrollBar>
#include <QLabel>
#include <QImageReader>
#include <QMessageBox>
#include <QGuiApplication>
#include <QDir>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QExposeEvent>

#include "Bitmap.h"

class ImageWidget : public QScrollArea {
public:

    QImage image;

    ImageWidget();
    ~ImageWidget();

    /* void setBitmap(Bitmap *bitmap); */
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    bool loadFile(QString fileName);
    void adjustSize();
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
private:
    Bitmap *bitmap;
    QLabel *imageLabel; 
    double scaleFactor = 1;
    QPoint mousePosition;
    bool isMiddleButtonDown = false;
};

#endif
