#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Bitmap.h"
#include "ImageWidget.h"
#include <QAbstractButton>
#include <QAction>
#include <QButtonGroup>
#include <QDockWidget>
#include <QFileDialog>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QKeySequence>
#include <QLabel>
#include <QLayout>
#include <QTreeView>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QStatusBar>

#define PALLETTE_LENGTH 28

class Editor : public QMainWindow
{
    Q_OBJECT

public:
    Editor();
    ~Editor();

private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void rotate();

    void setActiveColor(Color color);

private:

    ImageWidget *imageWidget;
    QButtonGroup *toolGroup;
    QButtonGroup *colorGroup;
    QTreeView *layerList;
    QStandardItemModel *layerListModel;
    Color pallette[PALLETTE_LENGTH] = {
        {0, 0, 0, 255},
        {255, 255, 255, 255},
        {128, 128, 128, 255},
        {192, 192, 192, 255},
        {128, 0, 0, 255},
        {255, 0, 0, 255},
        {128, 128, 0, 255},
        {255, 255, 0, 255},
        {0, 128, 0, 255},
        {0, 255, 0, 255},
        {0, 128, 128, 255},
        {0, 255, 255, 255},
        {0, 0, 128, 255},
        {0, 0, 255, 255},
        {128, 0, 128, 255},
        {255, 0, 255, 255},
        {128, 128, 64, 255},
        {255, 255, 128, 255},
        {0, 64, 64, 255},
        {0, 255, 128, 255},
        {0, 128, 255, 255},
        {128, 255, 255, 255},
        {0, 64, 128, 255},
        {128, 128, 255, 255},
        {128, 0, 255, 255},
        {255, 0, 128, 255},
        {128, 64, 0, 255},
        {255, 128, 64, 255},
    };

    void toolButtonClicked(QAbstractButton *button);
    void colorButtonClicked(QAbstractButton *button);
    void layerListSelectionChanged();
    void layerListModelUpdated(QStandardItem *item);

    void addLayer(Layer layer);

    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exitAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *normalSizeAction;
    QAction *fitToWindowAction;
    QAction *rotateAction;
};
#endif // MAINWINDOW_H
