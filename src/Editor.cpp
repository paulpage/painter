#include <QHBoxLayout>

#include "common.h"
#include "Editor.h"

Q_DECLARE_METATYPE(QDockWidget::DockWidgetFeatures)

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
    mimeTypeFilters.prepend("application/octet-stream");
    dialog->setMimeTypeFilters(mimeTypeFilters);
    if (mode == QFileDialog::AcceptSave)
        dialog->setDefaultSuffix("png");
}

Editor::Editor()
{
    qRegisterMetaType<QDockWidget::DockWidgetFeatures>();

    imageWidget = new ImageWidget(this);

    // Tools
    QDockWidget *leftDock = new QDockWidget;
    QWidget *leftContent = new QWidget;
    QWidget *toolWidget = new QFrame;
    QVBoxLayout *toolLayout = new QVBoxLayout(toolWidget);
    toolGroup = new QButtonGroup;
    toolGroup->setExclusive(true);
    QPushButton *toolButtons[5] = {
        new QPushButton("Pencil"),
        new QPushButton("Paintbrush"),
        new QPushButton("Color Picker"),
        new QPushButton("Paint Bucket"),
        new QPushButton("Spray Can"),
    };
    for (int i = 0; i < FINAL_TOOL_COUNT; i++) {
        toolLayout->addWidget(toolButtons[i]);
        toolGroup->addButton(toolButtons[i], i);
        toolButtons[i]->setCheckable(true);
    }
    toolButtons[0]->setChecked(true);
    connect(toolGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &Editor::toolButtonClicked);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftContent);
    leftLayout->addWidget(toolWidget);
    leftLayout->setAlignment(Qt::AlignTop);
    leftDock->setWidget(leftContent);

    // Colors
    QDockWidget *rightDock = new QDockWidget;
    QWidget *rightContent = new QWidget;
    QWidget *colorWidget = new QFrame;
    QVBoxLayout *colorLayout = new QVBoxLayout(colorWidget);
    colorGroup = new QButtonGroup;
    colorGroup->setExclusive(true);

    QPushButton *colorButtons[PALLETTE_LENGTH];
    for (int i = 0; i < PALLETTE_LENGTH; i++) {
        colorButtons[i] = new QPushButton;

        QPixmap pixmap(64, 64);
        pixmap.fill(QColor(pallette[i].r, pallette[i].g, pallette[i].b, pallette[i].a));
        QIcon icon(pixmap);
        colorButtons[i]->setIcon(icon);

        colorLayout->addWidget(colorButtons[i]);
        colorGroup->addButton(colorButtons[i], i);
        colorButtons[i]->setCheckable(true);
    }
    colorButtons[0]->setChecked(true);
    connect(colorGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &Editor::colorButtonClicked);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContent);
    rightLayout->addWidget(colorWidget);
    rightLayout->setAlignment(Qt::AlignTop);
    rightDock->setWidget(rightContent);

    // Layers
    layerList = new QListWidget;
    colorLayout->addWidget(layerList);
    
    setCentralWidget(imageWidget);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);

    // Menu Bar
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    newAction = fileMenu->addAction(tr("&New"), this, &Editor::newFile);
    newAction->setShortcut(QKeySequence::New);
    openAction = fileMenu->addAction(tr("&Open..."), this, &Editor::open);
    openAction->setShortcut(QKeySequence::Open);
    saveAction = fileMenu->addAction(tr("&Save"), this, &Editor::save);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setEnabled(false);
    saveAsAction = fileMenu->addAction(tr("Save &As..."), this, &Editor::saveAs);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setEnabled(false);
    fileMenu->addSeparator();
    exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    undoAction = editMenu->addAction(tr("&Undo"), this, &Editor::undo);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setEnabled(false);
    redoAction = editMenu->addAction(tr("&Redo"), this, &Editor::redo);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setEnabled(false);
    editMenu->addSeparator();
    cutAction = editMenu->addAction(tr("Cu&t"), this, &Editor::cut);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setEnabled(false);
    copyAction = editMenu->addAction(tr("&Copy"), this, &Editor::copy);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(false);
    pasteAction = editMenu->addAction(tr("&Paste"), this, &Editor::paste);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setEnabled(false);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    zoomInAction = viewMenu->addAction(tr("Zoom &In (25%)"), this, &Editor::zoomIn);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    zoomInAction->setEnabled(false);
    zoomOutAction = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &Editor::zoomOut);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    zoomOutAction->setEnabled(false);

    QMenu *imageMenu = menuBar()->addMenu(tr("&Image"));

    newFile();
}

void Editor::toolButtonClicked(QAbstractButton *button)
{
    imageWidget->activeTool = (Tool)toolGroup->id(button);
    button->setChecked(true);
}

void Editor::colorButtonClicked(QAbstractButton *button)
{
    int c = colorGroup->id(button);
    if (c >= 0 && c < PALLETTE_LENGTH) {
        imageWidget->activeColor = pallette[c];
        button->setChecked(true);
    }
}

void Editor::layerListSelectionChanged()
{
    layerList->selectedItems().first().name();
}

void Editor::newFile()
{
    Layer layer(800, 600);
    imageWidget->layers.append(layer);
    layerList->addItem(layer.name);
    imageWidget->setVisible(true);
    imageWidget->adjustSize();

    zoomInAction->setEnabled(true);
    zoomOutAction->setEnabled(true);
    saveAction->setEnabled(true);
    saveAsAction->setEnabled(true);
}

void Editor::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(&dialog, QFileDialog::AcceptOpen);

    QString fileName;
    if (dialog.exec() == QDialog::Accepted) {
        fileName = dialog.selectedFiles().at(0);
        if (imageWidget->loadFile(fileName)) {
            setWindowFilePath(fileName);
            zoomInAction->setEnabled(true);
            zoomOutAction->setEnabled(true);
            saveAction->setEnabled(true);
            saveAsAction->setEnabled(true);
            QString message = tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fileName));
            statusBar()->showMessage(message);
        }
    }
}

void Editor::save()
{
    QFileDialog dialog(this, tr("Save File"));
    initializeImageFileDialog(&dialog, QFileDialog::AcceptSave);
    QString fileName;
    if (dialog.exec() == QDialog::Accepted) {
        fileName = dialog.selectedFiles().at(0);
        bool write = true;
        QFile file(fileName);
        if (file.exists()) {
            QMessageBox confirmation;
            QString message = QString(tr("Are you sure you want to overwrite %1?")).arg(fileName);
            confirmation.setText(message);
            confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmation.setDefaultButton(QMessageBox::No);
            confirmation.setIcon(QMessageBox::Warning);
            write = (confirmation.exec() == QMessageBox::Yes);
        }
        if (write) {
            QImage image(
                    imageWidget->layers.first().bitmap.data,
                    imageWidget->layers.first().bitmap.width,
                    imageWidget->layers.first().bitmap.height,
                    imageWidget->layers.first().bitmap.width * 4,
                    QImage::Format_RGBA8888,
                    nullptr,
                    nullptr);
            image.save(fileName);
        }
    }
}

void Editor::saveAs()
{
}

void Editor::undo()
{
}

void Editor::redo()
{
}

void Editor::cut()
{
}

void Editor::copy()
{
}

void Editor::paste()
{
}

void Editor::zoomIn()
{
    imageWidget->scaleImage(1.25);
}

void Editor::zoomOut()
{
    imageWidget->scaleImage(0.8);
}

void Editor::normalSize()
{
}

void Editor::fitToWindow()
{
}

void Editor::setActiveColor(Color color)
{
    for (int i = 0; i < PALLETTE_LENGTH; i++) {
        if (color_eq(pallette[i], color)) {
            imageWidget->activeColor = pallette[i];
            colorGroup->button(i)->setChecked(true);
        }
    }
}

Editor::~Editor()
{
}
