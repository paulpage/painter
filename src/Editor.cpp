#include <QGridLayout>
#include <QItemSelectionModel>
#include <QClipboard>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QSpacerItem>

#include <lib/stb_ds.h>

#include "common.h"
#include "Image.h"
#include "Editor.h"

Q_DECLARE_METATYPE(QDockWidget::DockWidgetFeatures)

static void initializeImageFileDialog(QFileDialog *dialog, QFileDialog::AcceptMode mode) {
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

Editor::Editor() {
    qRegisterMetaType<QDockWidget::DockWidgetFeatures>();

    // Widgets
    // ============================================================

    tabs = new QTabWidget(this);

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
        new QPushButton("Line"),
        new QPushButton("Rectangle"),
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
    // TODO this doesn't seem to work anymore?
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
    setCentralWidget(tabs);
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
    saveAsAction = fileMenu->addAction(tr("Save &As..."), this, &Editor::saveAs);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    fileMenu->addSeparator();
    exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    undoAction = editMenu->addAction(tr("&Undo"), this, &Editor::undo);
    undoAction->setShortcut(QKeySequence::Undo);
    redoAction = editMenu->addAction(tr("&Redo"), this, &Editor::redo);
    redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addSeparator();
    cutAction = editMenu->addAction(tr("Cu&t"), this, &Editor::cut);
    cutAction->setShortcut(QKeySequence::Cut);
    copyAction = editMenu->addAction(tr("&Copy"), this, &Editor::copy);
    copyAction->setShortcut(QKeySequence::Copy);
    pasteAction = editMenu->addAction(tr("&Paste"), this, &Editor::paste);
    pasteAction->setShortcut(QKeySequence::Paste);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    zoomInAction = viewMenu->addAction(tr("Zoom &In (25%)"), this, &Editor::zoomIn);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    zoomOutAction = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &Editor::zoomOut);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);

    QMenu *imageMenu = menuBar()->addMenu(tr("&Image"));
    rotateAction = imageMenu->addAction(tr("&Rotate 90 degrees"), this, &Editor::rotate);
    flipHorizontalAction = imageMenu->addAction(tr("Flip &Horizontal"), this, &Editor::flipHorizontal);
    flipVerticalAction = imageMenu->addAction(tr("Flip &Vertical"), this, &Editor::flipVertical);

    QMenu *layerMenu = menuBar()->addMenu(tr("&Layer"));
    addLayerAction = layerMenu->addAction(tr("&Add layer"), this, &Editor::newLayer);
    addLayerAction->setShortcut(tr("Ctrl+Shift+N"));

    updateImageActions(false);

    // ============================================================

    createFile(800, 600);
}

ImageWidget *Editor::activeTab() {
    return static_cast<ImageWidget*>(tabs->currentWidget());
}

void Editor::toolButtonClicked(QAbstractButton *button) {
    activeTab()->activeTool = (Tool)toolGroup->id(button);
    button->setChecked(true);
}

void Editor::colorButtonClicked(QAbstractButton *button) {
    int c = colorGroup->id(button);
    if (c >= 0 && c < PALLETTE_LENGTH) {
        activeTab()->activeColor = pallette[c];
        button->setChecked(true);
    }
}

void Editor::layerListSelectionChanged() {
    int i = layerList->selectionModel()->selectedIndexes().first().row();
    activeTab()->activeLayerIndex = i;
}

void Editor::layerListModelUpdated(QStandardItem *item) {
    activeTab()->layerVisibilityMask[item->row()] = (item->checkState() == Qt::Checked);
    activeTab()->updateTextures();
}

void Editor::newFile() {
    auto dialog = new QDialog(this);
    auto layout = new QFormLayout(dialog);

    auto widthInput = new QLineEdit;
    auto heightInput = new QLineEdit;
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout->addRow(new QLabel(tr("Width (px):")), widthInput);
    layout->addRow(new QLabel(tr("Height (px):")), heightInput);
    layout->addRow(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this, widthInput, heightInput]{
        int width = widthInput->text().toInt();
        int height = heightInput->text().toInt();
        createFile(width, height);
    });
    dialog->show();
}

void Editor::newLayer() {
    if (activeTab()->isImageInitialized) {
        Layer layer = layer_create("Unnamed Layer", 0, 0, activeTab()->image.width, activeTab()->image.height);
        addLayer(layer);
    }
}

void Editor::createFile(int width, int height) {
    auto widget = new ImageWidget(this);
    widget->image = image_create(width, height);
    widget->tempLayer = layer_create("temp", 0, 0, width, height);
    widget->isImageInitialized = true;
    widget->filename = "UNNAMED";
    tabs->addTab(widget, widget->filename);
    tabs->setCurrentWidget(widget);
    Layer layer = layer_create("Unnamed Layer", 0, 0, width, height);
    addLayer(layer);

    activeTab()->setVisible(true);
    activeTab()->adjustSize();

    updateImageActions(true);
}

void Editor::open() {
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
            if (activeTab()->isImageInitialized) {
                image_free(activeTab()->image);
            }
            activeTab()->image = image_create(800, 600);
            activeTab()->tempLayer = layer_create("temp", 0, 0, 800, 600);
            activeTab()->isImageInitialized = true;
            activeTab()->scaleFactor = 1.0;
            activeTab()->updateTextures();
            activeTab()->setVisible(true);
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

void Editor::save() {
    if (activeTab()->filename.isNull()) {
        saveAs();
    } else {
        saveFile(activeTab()->filename);
    }
}

void Editor::saveAs() {
    QFileDialog dialog(this, tr("Save File"));
    initializeImageFileDialog(&dialog, QFileDialog::AcceptSave);
    if (dialog.exec() == QDialog::Accepted) {
        activeTab()->filename = dialog.selectedFiles().at(0);
        saveFile(activeTab()->filename);
    }
}

void Editor::undo() {
    image_undo(&activeTab()->image, &activeTab()->hist);
    activeTab()->updateTextures();
    refreshLayerList();
}

void Editor::redo() {
    image_redo(&activeTab()->image, &activeTab()->hist);
    activeTab()->updateTextures();
    refreshLayerList();
}

void Editor::cut() {}

void Editor::copy() {}

void Editor::paste() {
    QClipboard *clipboard = QApplication::clipboard();
    Layer layer = layerFromQImage(clipboard->image());
    if (layer.bitmap.width != 0) {
        addLayer(layer);
    }
}

void Editor::zoomIn() {
    activeTab()->scaleImage(1.25);
}

void Editor::zoomOut() {
    activeTab()->scaleImage(0.8);
}

void Editor::normalSize() {}

void Editor::fitToWindow() {}

void Editor::rotate() {
    activeTab()->rotate(90);
}

void Editor::flipHorizontal() {
}

void Editor::flipVertical() {
}

void Editor::setActiveColor(Color color) {
    for (int i = 0; i < PALLETTE_LENGTH; i++) {
        if (color_eq(pallette[i], color)) {
            activeTab()->activeColor = pallette[i];
            colorGroup->button(i)->setChecked(true);
        }
    }
}

void Editor::refreshLayerList() {
    layerListModel->clear();
    for (int i = 0; i < arrlen(activeTab()->image.layers); i++) {
        QStandardItem *item = new QStandardItem();
        item->setText(activeTab()->image.layers[i].name);
        item->setCheckable(activeTab()->layerVisibilityMask[i]);
        item->setCheckState(Qt::Checked);
        item->setUserTristate(false);
        item->setEditable(true); // TODO change layer name based on editing
        layerListModel->setItem(layerListModel->rowCount(), item);
        layerList->selectionModel()->select(layerListModel->indexFromItem(item), QItemSelectionModel::SelectionFlags(QItemSelectionModel::ClearAndSelect));
    }
}

void Editor::addLayer(Layer layer) {
    image_add_layer(&activeTab()->image, layer);
    arrput(activeTab()->layerVisibilityMask, true);
    activeTab()->activeLayerIndex = arrlen(activeTab()->image.layers) - 1;
    // TODO if this gets undone the visibility mask won't get undone. Come to think of it, we could probably just add is_visible to the layer. Also need to update the layer list when we undo.
    image_take_snapshot(&activeTab()->image, &activeTab()->hist);
    refreshLayerList();
}

void Editor::updateImageActions(bool enabled) {
    // TODO disable undo and redo when there's no available history
    saveAction->setEnabled(enabled);
    saveAsAction->setEnabled(enabled);
    undoAction->setEnabled(enabled);
    redoAction->setEnabled(enabled);
    cutAction->setEnabled(enabled);
    copyAction->setEnabled(enabled);
    pasteAction->setEnabled(enabled);
    zoomInAction->setEnabled(enabled);
    zoomOutAction->setEnabled(enabled);
    rotateAction->setEnabled(enabled);
    addLayerAction->setEnabled(enabled);
}

void Editor::saveFile(QString filename) {
    bool write = true;
    QFile file(filename);
    if (file.exists()) {
        QMessageBox confirmation;
        QString message = QString(tr("Are you sure you want to overwrite %1?")).arg(filename);
        confirmation.setText(message);
        confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        confirmation.setDefaultButton(QMessageBox::No);
        confirmation.setIcon(QMessageBox::Warning);
        write = (confirmation.exec() == QMessageBox::Yes);
    }
    if (write) {
        QImage image(
                activeTab()->bitmap.data,
                activeTab()->bitmap.width,
                activeTab()->bitmap.height,
                activeTab()->bitmap.width * 4,
                QImage::Format_RGBA8888,
                nullptr,
                nullptr);
        image.save(filename);
    }
}

Editor::~Editor() {}
