#include "ImageEditor.h"

static void initializeImageFileDialog(QFileDialog *dialog, QFileDialog::AcceptMode mode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog->setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    QByteArrayList supportedMimeTypes = mode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog->setMimeTypeFilters(mimeTypeFilters);
    dialog->selectMimeTypeFilter("image/jpeg");
    if (mode == QFileDialog::AcceptSave)
        dialog->setDefaultSuffix("jpg");
}

ImageEditor::ImageEditor(QWidget *parent)
    : QMainWindow(parent), imageWidget(new ImageWidget)
{
    setCentralWidget(imageWidget);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    newAction = fileMenu->addAction(tr("&New"), this, &ImageEditor::newFile);
    newAction->setShortcut(QKeySequence::New);
    openAction = fileMenu->addAction(tr("&Open..."), this, &ImageEditor::open);
    openAction->setShortcut(QKeySequence::Open);
    saveAction = fileMenu->addAction(tr("&Save"), this, &ImageEditor::save);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setEnabled(false);
    saveAsAction = fileMenu->addAction(tr("Save &As..."), this, &ImageEditor::saveAs);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setEnabled(false);
    fileMenu->addSeparator();
    exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    undoAction = editMenu->addAction(tr("&Undo"), this, &ImageEditor::undo);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setEnabled(false);
    redoAction = editMenu->addAction(tr("&Redo"), this, &ImageEditor::redo);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setEnabled(false);
    editMenu->addSeparator();
    cutAction = editMenu->addAction(tr("Cu&t"), this, &ImageEditor::cut);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setEnabled(false);
    copyAction = editMenu->addAction(tr("&Copy"), this, &ImageEditor::copy);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(false);
    pasteAction = editMenu->addAction(tr("&Paste"), this, &ImageEditor::paste);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setEnabled(false);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    zoomInAction = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageEditor::zoomIn);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    zoomInAction->setEnabled(false);
    zoomOutAction = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageEditor::zoomOut);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    zoomOutAction->setEnabled(false);

    QMenu *imageMenu = menuBar()->addMenu(tr("&Image"));
}

void ImageEditor::newFile()
{
    /* Bitmap bitmap = bitmap_create(100, 100); */
    /* QImage image(bitmap.data, bitmap.width, bitmap.height, bitmap.width * 4, QImage::Format_RGBA8888, nullptr, nullptr); */
    /* imageWidget->image = image; */
    imageWidget->bitmap = bitmap_create(100, 100);
    imageWidget->setVisible(true);
    imageWidget->adjustSize();

    zoomInAction->setEnabled(true);
    zoomOutAction->setEnabled(true);
    saveAction->setEnabled(true);
    saveAsAction->setEnabled(true);
}

void ImageEditor::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(&dialog, QFileDialog::AcceptOpen);

    QString fileName;
    if (dialog.exec() == QDialog::Accepted) {
        fileName = dialog.selectedFiles().first();
        if (imageWidget->loadFile(fileName)) {
            setWindowFilePath(fileName);
        }
    }

    zoomInAction->setEnabled(true);
    zoomOutAction->setEnabled(true);
    saveAction->setEnabled(true);
    saveAsAction->setEnabled(true);

    QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(imageWidget->image.width()).arg(imageWidget->image.height()).arg(imageWidget->image.depth());
    statusBar()->showMessage(message);
    /* re-init to change texture, TODO change this */
    imageWidget->initializeGL();
}

void ImageEditor::save()
{
}

void ImageEditor::saveAs()
{
}

void ImageEditor::undo()
{
}

void ImageEditor::redo()
{
}

void ImageEditor::cut()
{
}

void ImageEditor::copy()
{
}

void ImageEditor::paste()
{
}

void ImageEditor::zoomIn()
{
    imageWidget->scaleImage(1.25);
}

void ImageEditor::zoomOut()
{
    imageWidget->scaleImage(0.8);
}

void ImageEditor::normalSize()
{
}

void ImageEditor::fitToWindow()
{
}

ImageEditor::~ImageEditor()
{
}

// ========================================

