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
#include <QLineEdit>
#include <QTabWidget>

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
    void flipHorizontal();
    void flipVertical();
    void newLayer();

    void setActiveColor(Color color);

private:

    QTabWidget *tabs;
    QButtonGroup *toolGroup;
    QButtonGroup *colorGroup;
    QTreeView *layerList;
    QStandardItemModel *layerListModel;
    ImageWidget *activeTab();
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
    void refreshLayerList();
    void layerListModelUpdated(QStandardItem *item);
    void createFile(int width, int height);
    void addLayer(Layer layer);
    void updateImageActions(bool enabled);
    void saveFile(QString filename);

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
    QAction *flipHorizontalAction;
    QAction *flipVerticalAction;
    QAction *addLayerAction;
};
#endif // MAINWINDOW_H
