#include <QGridLayout>
#include <QItemSelectionModel>
#include <QClipboard>
#include <QApplication>

#include <lib/stb_ds.h>

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

static Layer layerFromQImage(QImage image) {
    if (!image.isNull()) {

        image = image.convertToFormat(QImage::Format_RGBA8888);
        int width = image.width();
        int height = image.height();
        Bitmap bitmap = bitmap_create(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                QRgb c = image.pixel(x, y);
                Color color = {
                    (unsigned char)qRed(c),
                    (unsigned char)qGreen(c),
                    (unsigned char)qBlue(c),
                    (unsigned char)qAlpha(c),
                };
                bitmap_draw_pixel(&bitmap, x, y, color);
            }
        }

        Layer layer = layer_create_from_bitmap("Unnamed Layer", 100, 100, bitmap);
        return layer;
    }
    return layer_create("Unnamed Layer", 0, 0, 0, 0);
}

Editor::Editor()
{
    qRegisterMetaType<QDockWidget::DockWidgetFeatures>();

    // Widgets
    // ============================================================

    imageWidget = new ImageWidget(this);

    // Tools
    QWidget *toolWidget = new QWidget;
    QVBoxLayout *toolLayout = new QVBoxLayout(toolWidget);
    toolGroup = new QButtonGroup;
    toolGroup->setExclusive(true);
    QPushButton *toolButtons[FINAL_TOOL_COUNT] = {
        new QPushButton("Pencil"),
        new QPushButton("Paintbrush"),
        new QPushButton("Color Picker"),
        new QPushButton("Paint Bucket"),
        new QPushButton("Spray Can"),
        new QPushButton("Eraser"),
        new QPushButton("Move"),
        new QPushButton("Rectangle Select"),
    };
    for (int i = 0; i < FINAL_TOOL_COUNT; i++) {
        toolLayout->addWidget(toolButtons[i]);
        toolGroup->addButton(toolButtons[i], i);
        toolButtons[i]->setCheckable(true);
    }
    toolButtons[0]->setChecked(true);
    connect(toolGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &Editor::toolButtonClicked);

    // Colors
    QWidget *colorWidget = new QWidget;
    QGridLayout *colorLayout = new QGridLayout(colorWidget);
    colorLayout->setHorizontalSpacing(2);
    colorLayout->setVerticalSpacing(2);
    colorGroup = new QButtonGroup;
    colorGroup->setExclusive(true);

    QPushButton *colorButtons[PALLETTE_LENGTH];
    for (int i = 0; i < PALLETTE_LENGTH; i++) {
        colorButtons[i] = new QPushButton;
        colorButtons[i]->setFlat(true);
        colorButtons[i]->setFixedSize(QSize(colorButtons[i]->sizeHint().height(), colorButtons[i]->sizeHint().height()));

        QPixmap pixmap(64, 64);
        pixmap.fill(QColor(pallette[i].r, pallette[i].g, pallette[i].b, pallette[i].a));
        QIcon icon(pixmap);
        colorButtons[i]->setIcon(icon);

        colorLayout->addWidget(colorButtons[i], i / 2, i % 2);
        colorGroup->addButton(colorButtons[i], i);
        colorButtons[i]->setCheckable(true);
    }
    colorButtons[0]->setChecked(true);
    connect(colorGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &Editor::colorButtonClicked);

    // Layers
    layerList = new QTreeView;
    layerList->setRootIsDecorated(false);

    layerListModel = new QStandardItemModel(0, 1, this);
    layerListModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Layers"));
    layerList->setModel(layerListModel);

    connect(layerList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &Editor::layerListSelectionChanged);
    connect(layerListModel, &QStandardItemModel::itemChanged, this, &Editor::layerListModelUpdated);

    // Layout
    // ============================================================

    // Left dock
    QDockWidget *leftDock = new QDockWidget;
    QWidget *leftContent = new QWidget;
    QWidget *leftWidget = new QFrame;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftContent);
    leftLayout->addWidget(leftWidget);
    leftLayout->setAlignment(Qt::AlignTop);
    leftDock->setWidget(leftContent);

    // Right dock
    QDockWidget *rightDock = new QDockWidget;
    QWidget *rightContent = new QWidget;
    QWidget *rightWidget = new QFrame;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContent);
    rightLayout->addWidget(rightWidget);
    rightLayout->setAlignment(Qt::AlignTop);
    rightDock->setWidget(rightContent);

    // Widget placement
    leftLayout->addWidget(toolWidget);
    leftLayout->addWidget(colorWidget);
    rightLayout->addWidget(layerList);

    // Dock and central widget placement
    setCentralWidget(imageWidget);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);

    // Menu Bar
    // ============================================================

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
    rotateAction = imageMenu->addAction(tr("Rotate 90 degrees"), this, &Editor::rotate);
    rotateAction->setEnabled(false);

    // ============================================================

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
    int i = layerList->selectionModel()->selectedIndexes().first().row();
    imageWidget->activeLayerIndex = i;
}

void Editor::layerListModelUpdated(QStandardItem *item)
{
    imageWidget->layerVisibilityMask[item->row()] = (item->checkState() == Qt::Checked);
    imageWidget->updateTextures();
}

void Editor::newFile()
{

    if (imageWidget->isImageInitialized) {
        image_free(imageWidget->image);
    }
    imageWidget->image = image_create(800, 600, "UNNAMED");
    imageWidget->isImageInitialized = true;
    Layer layer = layer_create("Unnamed Layer", 0, 0, 800, 600);
    addLayer(layer);

    imageWidget->setVisible(true);
    imageWidget->adjustSize();

    zoomInAction->setEnabled(true);
    zoomOutAction->setEnabled(true);
    saveAction->setEnabled(true);
    saveAsAction->setEnabled(true);

    cutAction->setEnabled(true);
    copyAction->setEnabled(true);
    pasteAction->setEnabled(true);

    rotateAction->setEnabled(true);
}

void Editor::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(&dialog, QFileDialog::AcceptOpen);

    QString fileName;
    if (dialog.exec() == QDialog::Accepted) {
        fileName = dialog.selectedFiles().at(0);
        QImageReader reader(fileName);
        reader.setAutoTransform(true);
        QImage image = reader.read();
        Layer layer = layerFromQImage(image);
        if (layer.bitmap.width != 0) {
            if (imageWidget->isImageInitialized) {
                image_free(imageWidget->image);
            }
            imageWidget->image = image_create(800, 600, "UNNAMED");
            imageWidget->isImageInitialized = true;
            imageWidget->scaleFactor = 1.0;
            imageWidget->updateTextures();
            imageWidget->setVisible(true);
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
                    imageWidget->bitmap.data,
                    imageWidget->bitmap.width,
                    imageWidget->bitmap.height,
                    imageWidget->bitmap.width * 4,
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
    QClipboard *clipboard = QApplication::clipboard();
    Layer layer = layerFromQImage(clipboard->image());
    if (layer.bitmap.width != 0) {
        addLayer(layer);
    }
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

void Editor::rotate()
{
    imageWidget->rotate(90);
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

void Editor::addLayer(Layer layer)
{
    if (imageWidget->isImageInitialized) {
        imageWidget->image = image_create(layer.bitmap.width, layer.bitmap.height, "UNNAMED");
        imageWidget->isImageInitialized = true;
    }
    image_add_layer(&imageWidget->image, layer);
    arrput(imageWidget->layerVisibilityMask, true);
    imageWidget->activeLayerIndex = arrlen(imageWidget->image.layers) - 1;

    QStandardItem *item = new QStandardItem();
    item->setText(layer.name);
    item->setCheckable(true);
    item->setCheckState(Qt::Checked);
    item->setUserTristate(false);
    item->setEditable(true); // TODO change layer name based on editing
    layerListModel->setItem(layerListModel->rowCount(), item);
    // TODO figure out the right syntax to select the new layer
    layerList->selectionModel()->select(layerListModel->indexFromItem(item), QItemSelectionModel::SelectionFlags(QItemSelectionModel::ClearAndSelect));
}

Editor::~Editor()
{
}
