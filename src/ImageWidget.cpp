#include <QRandomGenerator>
#include <math.h>

#include "ImageWidget.h"

ImageWidget::ImageWidget(QWidget *parent)
{
    bitmap = bitmap_create(100, 100);
    QImage image(bitmap.data, bitmap.width, bitmap.height, bitmap.width * 4, QImage::Format_RGBA8888, nullptr, nullptr);
    texture = new QOpenGLTexture(image);
    setBackgroundRole(QPalette::Dark);
    timer = new QTimer(this);
    timer->setInterval(20);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&ImageWidget::useSprayCan));
    connect(this, SIGNAL(sendColorChanged(Color)), parent, SLOT(setActiveColor(Color)));
}

QPoint ImageWidget::globalToBitmap(QPoint g)
{
    QPoint base = g - mapToGlobal(QPoint(0, 0));
    double bitmapWidth = scaleFactor * (double)bitmap.width;
    double bitmapStartX = (double)width() / 2 - bitmapWidth / 2 + offsetX;
    double screenDistanceX = (double)base.x() - bitmapStartX;
    int bx = (int)(screenDistanceX / scaleFactor);

    double bitmapHeight = scaleFactor * (double)bitmap.height;
    double bitmapStartY = (double)height() / 2 - bitmapWidth / 2 + offsetY;
    double screenDistanceY = (double)base.y() - bitmapStartY;
    int by = (int)(screenDistanceY / scaleFactor);

    return QPoint(bx, by);
}

void ImageWidget::scaleImage(double factor)
{
    scaleFactor *= factor;
    updateTexture();

    /* adjustScrollBar(horizontalScrollBar(), factor); */
    /* adjustScrollBar(verticalScrollBar(), factor); */
}

void ImageWidget::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    /* scrollBar->setValue(int(factor * scrollBar->value() */
    /*                         + ((factor - 1) * scrollBar->pageStep()/2))); */
}

bool ImageWidget::loadFile(QString fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Can't load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
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
            /* bitmap_set_pixel(&bitmap, x, y, color); */
        }
    }

    scaleFactor = 1.0;
    updateTexture();
    setVisible(true);

    return true;
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

    applyTools();

    updateTexture();
    event->accept();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *event) {
    isMiddleButtonDown = !((event->button() & Qt::MidButton) == Qt::MidButton);
    isLeftButtonDown = !((event->button() & Qt::LeftButton) == Qt::LeftButton);
    if (!isLeftButtonDown) {
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
        /* offsetX += (GLfloat)diff.x() * 2 / (GLfloat)width(); */
        /* offsetY += (GLfloat)diff.y() * 2 / (GLfloat)height(); */
        /* imageLabel->move(imageLabel->pos() + diff); */
    }

    applyTools();

    lastMousePosition = mousePosition;
    mousePosition = event->globalPos();
    event->accept();
}

void ImageWidget::initializeGL()
{
    initializeOpenGLFunctions();

    updateTexture();

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

void ImageWidget::updateTexture() {
    if (isValid()) {
        QImage image(bitmap.data, bitmap.width, bitmap.height, bitmap.width * 4, QImage::Format_RGBA8888, nullptr, nullptr);
        delete texture;
        texture = new QOpenGLTexture(image);
        texture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    }
}

void ImageWidget::paintGL()
{
    QOpenGLBuffer vbo;
    GLfloat xRatio = (GLfloat)scaleFactor * (GLfloat)bitmap.width / (GLfloat)width();
    GLfloat yRatio = (GLfloat)scaleFactor * (GLfloat)bitmap.height / (GLfloat)height();
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

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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
    texture->bind();
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

void ImageWidget::applyTools()
{
    if (isLeftButtonDown) {
        QPoint lastPixelPosition = globalToBitmap(lastMousePosition);
        QPoint pixelPosition = globalToBitmap(mousePosition);
        switch (activeTool) {
            case TOOL_PENCIL:
                bitmap_draw_line(
                        &bitmap,
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
                                    &bitmap,
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
                if (bitmap_get_pixel(&bitmap, pixelPosition.x(), pixelPosition.y(), &color)) {
                    activeColor = color;
                    emit sendColorChanged(color);
                }
                break;
            case TOOL_PAINT_BUCKET:
                        bitmap_fill(
                                &bitmap,
                                pixelPosition.x(),
                                pixelPosition.y(),
                                activeColor);
                break;
            case TOOL_SPRAY_CAN:
                /* useSprayCan(); */
                if (!timer->isActive()) {
                    timer->start();
                }
                break;
            default:
                break;
        }
        updateTexture();
        update();
    }
}

void ImageWidget::useSprayCan()
{
    QPoint pixelPosition = globalToBitmap(mousePosition);
    int x = pixelPosition.x();
    int y = pixelPosition.y();
    for (int i = 0; i < 20; i++) {
        int dx = QRandomGenerator::global()->bounded(-20, 20);
        int dy = QRandomGenerator::global()->bounded(-20, 20);
        if (sqrt((double)(dx * dx) + (double)(dy * dy)) < 20.0f) {
            bitmap_draw_pixel(&bitmap, x + dx, y + dy, activeColor);
        }
    }
    updateTexture();
    update();
}
