#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QColor>
#include <QDir>
#include <QElapsedTimer>
#include <QExposeEvent>
#include <QGuiApplication>
#include <QImageReader>
#include <QList>
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QString>
#include <QTimer>
#include <QVector>
#include <QWheelEvent>

#include "Bitmap.h"
#include "Layer.h"
#include "common.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture);

class ImageWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    using QOpenGLWidget::QOpenGLWidget;

    ImageWidget(QWidget *parent);
    ~ImageWidget();

    QPoint globalToLayer(Layer *layer, QPoint g);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    bool loadFile(QString fileName);
    void scaleImage(double factor);
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void updateTextures();
    Bitmap bitmap = bitmap_create(0, 0);

    Layer *activeLayer;
    QVector<Layer> layers;
    Tool activeTool = TOOL_PENCIL;
    Color activeColor = {255, 0, 0, 255};

signals:
    void sendColorChanged(Color color);

private:
    QTimer *timer;
    QElapsedTimer *eTimer;
    double scaleFactor = 1;
    QPoint mousePosition;
    QPoint lastMousePosition;
    int offsetX = 0;
    int offsetY = 0;
    QOpenGLTexture *texture;
    bool isMiddleButtonDown = false;
    bool isLeftButtonDown = false;
    QOpenGLShaderProgram *program;
    GLuint textureId = 0;


    void useSprayCan();
    void applyTools(QMouseEvent *event);
};

#endif
