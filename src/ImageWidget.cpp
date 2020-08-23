#include "ImageWidget.h"

ImageWidget::ImageWidget(): imageLabel(new QLabel)
{
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    setBackgroundRole(QPalette::Dark);
    setWidget(imageLabel);
    setVisible(false);
}

void ImageWidget::scaleImage(double factor)
{
    scaleFactor *= factor;

    // Resize moves the widget back to (0, 0), so restore it.
    QPoint oldPosition = imageLabel->pos();
    imageLabel->resize(scaleFactor * imageLabel->pixmap(Qt::ReturnByValue).size());
    imageLabel->move(oldPosition);

    adjustScrollBar(horizontalScrollBar(), factor);
    adjustScrollBar(verticalScrollBar(), factor);
}

void ImageWidget::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageWidget::adjustSize()
{
    imageLabel->adjustSize();
}


/* void ImageEditor::setBitmap(Bitmap *bitmap) { */
/*     QImage image(bitmap->data, bitmap->width, bitmap->height, bitmap->width * 4, QImage::Format_RGBA8888, nullptr, nullptr); */
/*     imageLabel->setPixmap(QPixmap::fromImage(image)); */
/*     scaleFactor = 1.0; */
/*     scrollArea->setVisible(true); */

    /* saveAction->setEnabled(true); */
    /* saveAsAction->setEnabled(true); */
    /* cutAction->setEnabled(true); */
    /* copyAction->setEnabled(true); */
/* } */

bool ImageWidget::loadFile(QString fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    image = reader.read();
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Can't load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;
    setVisible(true);
    imageLabel->adjustSize();

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
    QScrollArea::mousePressEvent(event);
    mousePosition = event->globalPos();
    isMiddleButtonDown = ((event->button() & Qt::MidButton) == Qt::MidButton);
    printf("Mouse position: %d, %d\n", mousePosition.x(), mousePosition.y());
    event->accept();
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    QScrollArea::mouseMoveEvent(event);

    if (isMiddleButtonDown) {
        QPoint diff = event->globalPos() - mousePosition;
        imageLabel->move(imageLabel->pos() + diff);
    }

    mousePosition = event->globalPos();
    printf("Mouse position: %d, %d\n", mousePosition.x(), mousePosition.y());
    event->accept();
}

ImageWidget::~ImageWidget()
{
}
