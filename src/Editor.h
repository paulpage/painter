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
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QStatusBar>

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

    void setActiveColor(Color color);

private:

    ImageWidget *imageWidget;
    QButtonGroup *toolGroup;
    QButtonGroup *colorGroup;
    QListWidget *layerList;
    Color pallette[5] = {
        {255, 0, 0, 255},
        {0, 255, 0, 255},
        {0, 0, 255, 255},
        {0, 0, 0, 255},
        {255, 255, 255, 255},
    };

    void toolButtonClicked(QAbstractButton *button);
    void colorButtonClicked(QAbstractButton *button);

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
};
#endif // MAINWINDOW_H
