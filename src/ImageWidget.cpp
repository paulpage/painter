#include <QRandomGenerator>
#include <math.h>

#include "lib/stb_ds.h"

#include "ImageWidget.h"

ImageWidget::ImageWidget(QWidget *parent)
{
    setBackgroundRole(QPalette::Dark);
    timer = new QTimer(this);
    eTimer = new QElapsedTimer;
    eTimer->start();
    timer->setInterval(5);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&ImageWidget::useSprayCan));
    connect(this, SIGNAL(sendColorChanged(Color)), parent, SLOT(setActiveColor(Color)));
}

QPoint ImageWidget::globalToCanvas(QPoint g)
{
    QPoint base = g - mapToGlobal(QPoint(0, 0));
    double layerWidth = scaleFactor * image.width;
    double layerStartX = (double)width() / 2 - layerWidth / 2 + offsetX;
    double screenDistanceX = (double)base.x() - layerStartX;
    int bx = (int)(screenDistanceX / scaleFactor);

    double layerHeight = scaleFactor * image.height;
    double layerStartY = (double)height() / 2 - layerHeight / 2 + offsetY;
    double screenDistanceY = (double)base.y() - layerStartY;
    int by = (int)(screenDistanceY / scaleFactor);

    return QPoint(bx, by);
}

void ImageWidget::scaleImage(double factor)
{
    scaleFactor *= factor;
    updateTextures();

    /* adjustScrollBar(horizontalScrollBar(), factor); */
    /* adjustScrollBar(verticalScrollBar(), factor); */
}

void ImageWidget::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    /* scrollBar->setValue(int(factor * scrollBar->value() */
    /*                         + ((factor - 1) * scrollBar->pageStep()/2))); */
}

void ImageWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull()) {
        scaleImage(1.0f + (numDegrees.y() / 100.0f));
    }

    // TODO: scroll with pixels if it's supported. Caveat: We can check numPixels for null,
    // but the value is unreliable on X11.
    /* if (!numPixels.isNull()) { */
    /*     scrollWithPixels(numPixels); */
    /* } else if (!numDegrees.isNull()) { */
    /*     QPoint numSteps = numDegrees / 15; */
    /*     scrollWithDegrees(numSteps); */
    /* } */

    event->accept();
}

void ImageWidget::mousePressEvent(QMouseEvent *event) {
    // We have to handle this event rather that simply checking the button
    // state during mouse move because the mouse move event (on my system)
    // is not sent when there are no buttons down.
    QOpenGLWidget::mousePressEvent(event);
    lastMousePosition = event->globalPos();
    mousePosition = event->globalPos();
    isMiddleButtonDown = ((event->button() & Qt::MidButton) == Qt::MidButton);
    isLeftButtonDown = ((event->button() & Qt::LeftButton) == Qt::LeftButton);

    applyTools(event);

    updateTextures();
    event->accept();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *event) {
    isMiddleButtonDown = !((event->button() & Qt::MidButton) == Qt::MidButton);
    isLeftButtonDown = !((event->button() & Qt::LeftButton) == Qt::LeftButton);
    if (!isLeftButtonDown) {
        image_take_snapshot(&image, &hist);
        timer->stop();
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseMoveEvent(event);

    if (isMiddleButtonDown) {
        QPoint diff = event->globalPos() - mousePosition;
        offsetX += diff.x();
        offsetY += diff.y();
    }

    applyTools(event);

    lastMousePosition = mousePosition;
    mousePosition = event->globalPos();

    event->accept();
}

void ImageWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    updateTextures();

    QOpenGLShader *vertShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vertSrc =
        "attribute highp vec2 vertex;\n"
        "attribute mediump vec2 texCoord;\n"
        "varying mediump vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = vec4(vertex, 0, 1);\n"
        "    texc = texCoord;\n"
        "}\n";
    if (!vertShader->compileSourceCode(vertSrc)) {
        printf("Failed to compile vertex shader!\n");
    }

    QOpenGLShader *fragShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fragSrc =
        "uniform sampler2D texture;\n"
        "varying mediump vec2 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texc);\n"
        "}\n";
    if (!fragShader->compileSourceCode(fragSrc)) {
        printf("Failed to compile fragment shader!\n");
    }

    program = new QOpenGLShaderProgram;
    program->addShader(vertShader);
    program->addShader(fragShader);
    program->bindAttributeLocation("vertex", 0);
    program->bindAttributeLocation("texCoord", 1);
    program->link();
    program->bind();

    program->setUniformValue("texture", 0);
}

void ImageWidget::updateTextures() {
    if (isValid()) {
        bitmap_free(&bitmap);
        bitmap = bitmap_create(image.width, image.height);
        for (int i = 0; i < arrlen(image.layers); i++) {
            if (layerVisibilityMask[i]) {
                bitmap_blend(&bitmap, &image.layers[i].bitmap, image.layers[i].x, image.layers[i].y);
            }
        }

        glEnable(GL_TEXTURE_2D);

        glDeleteTextures(1, &textureId); // This is safe to do because glDeleteTextures ignores 0
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        QImage image(bitmap.data, bitmap.width, bitmap.height, bitmap.width * 4, QImage::Format_RGBA8888, nullptr, nullptr);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glDisable(GL_TEXTURE_2D);
    }
}

void ImageWidget::paintGL()
{
    QOpenGLBuffer vbo;

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLfloat xRatio = (GLfloat)scaleFactor * (GLfloat)image.width / (GLfloat)width();
    GLfloat yRatio = (GLfloat)scaleFactor * (GLfloat)image.height / (GLfloat)height();
    GLfloat scaledOffsetX = (GLfloat)offsetX * 2 / (GLfloat)width();
    GLfloat scaledOffsetY = (GLfloat)offsetY * 2 / (GLfloat)height();
    GLfloat vertData[24] = {
        -xRatio + scaledOffsetX, -yRatio - scaledOffsetY, 0, 1,
        +xRatio + scaledOffsetX, -yRatio - scaledOffsetY, 1, 1,
        +xRatio + scaledOffsetX, +yRatio - scaledOffsetY, 1, 0,
        -xRatio + scaledOffsetX, -yRatio - scaledOffsetY, 0, 1,
        +xRatio + scaledOffsetX, +yRatio - scaledOffsetY, 1, 0,
        -xRatio + scaledOffsetX, +yRatio - scaledOffsetY, 0, 0,
    };
    vbo.create();
    vbo.bind();
    vbo.allocate(vertData, 24 * sizeof(GLfloat));

    program->enableAttributeArray(0);
    program->setAttributeBuffer(
            0,
            GL_FLOAT,
            0,
            2,
            4 * sizeof(GLfloat));
    program->enableAttributeArray(1);
    program->setAttributeBuffer(
            1,
            GL_FLOAT,
            2 * sizeof(GLfloat),
            2,
            4 * sizeof(GLfloat));
    glBindTexture(GL_TEXTURE_2D, textureId);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    update();
}

void ImageWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::applyTools(QMouseEvent *event)
{
    if (isLeftButtonDown) {
        QPoint lastPixelPosition = globalToCanvas(lastMousePosition) - QPoint(image.layers[activeLayerIndex].x, image.layers[activeLayerIndex].y);
        QPoint pixelPosition = globalToCanvas(mousePosition) - QPoint(image.layers[activeLayerIndex].x, image.layers[activeLayerIndex].y);
        switch (activeTool) {
            case TOOL_PENCIL:
                bitmap_draw_line(
                        &image.layers[activeLayerIndex].bitmap,
                        lastPixelPosition.x(),
                        lastPixelPosition.y(),
                        pixelPosition.x(),
                        pixelPosition.y(),
                        activeColor);
                break;
            case TOOL_PAINTBRUSH:
                for (int y = -5; y < 5; y++) {
                    for (int x = -5; x < 5; x++) {
                        if (sqrt((double)(x * x) + (double)(y * y)) < 5.0f) {
                            bitmap_draw_line(
                                    &image.layers[activeLayerIndex].bitmap,
                                    lastPixelPosition.x() + x,
                                    lastPixelPosition.y() + y,
                                    pixelPosition.x() + x,
                                    pixelPosition.y() + y,
                                    activeColor);
                        }
                    }
                }
                break;
            case TOOL_COLOR_PICKER:
                Color color;
                if (bitmap_get_pixel(&image.layers[activeLayerIndex].bitmap, pixelPosition.x(), pixelPosition.y(), &color)) {
                    activeColor = color;
                    emit sendColorChanged(color);
                }
                break;
            case TOOL_PAINT_BUCKET:
                        bitmap_fill(
                                &image.layers[activeLayerIndex].bitmap,
                                pixelPosition.x(),
                                pixelPosition.y(),
                                activeColor);
                break;
            case TOOL_SPRAY_CAN:
                if (!timer->isActive()) {
                    timer->start();
                }
                break;
            case TOOL_ERASER:
                for (int y = -5; y < 5; y++) {
                    for (int x = -5; x < 5; x++) {
                        if (sqrt((double)(x * x) + (double)(y * y)) < 5.0f) {
                            bitmap_draw_line(
                                    &image.layers[activeLayerIndex].bitmap,
                                    lastPixelPosition.x() + x,
                                    lastPixelPosition.y() + y,
                                    pixelPosition.x() + x,
                                    pixelPosition.y() + y,
                                    Color { 0, 0, 0, 0 });
                        }
                    }
                }
                break;
            case TOOL_MOVE:
                {
                    QPoint diff = event->globalPos() - mousePosition;
                    QPoint newPosition = globalToCanvas(event->globalPos() - diff);
                    image.layers[activeLayerIndex].x = newPosition.x();
                    image.layers[activeLayerIndex].y = newPosition.y();
                }
                break;
            case TOOL_RECTANGLE_SELECT:
                break;
            default:
                break;
        }
        updateTextures();
        update();
    }
}

void ImageWidget::useSprayCan()
{
    QPoint pixelPosition = globalToCanvas(mousePosition);
    int x = pixelPosition.x();
    int y = pixelPosition.y();
    for (int i = 0; i < 20; i++) {
        int dx = QRandomGenerator::global()->bounded(-20, 20);
        int dy = QRandomGenerator::global()->bounded(-20, 20);
        if (sqrt((double)(dx * dx) + (double)(dy * dy)) < 20.0f) {
            bitmap_draw_pixel(&image.layers[activeLayerIndex].bitmap, x + dx, y + dy, activeColor);
        }
    }
    updateTextures();
    update();
}

void ImageWidget::rotate(int degrees)
{
    switch (degrees) {
        case 90:
            {
                int oldY = image.height;
                for (int i = 0; i < arrlen(image.layers); i++) {
                    Bitmap newBitmap = bitmap_create_rotated(&image.layers[i].bitmap);
                    layer_free(&image.layers[i]);
                    Layer layer = layer_create_from_bitmap(
                            image.layers[i].name,
                            oldY - image.layers[i].y - image.layers[i].bitmap.height,
                            image.layers[i].x,
                            newBitmap);
                    image.layers[i] = layer;
                }
                updateTextures();
            }
            break;
        default:
            break;
    }
}
