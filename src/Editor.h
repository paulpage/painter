#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Bitmap.h"
#include "ImageWidget.h"
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

enum Tool {
    PENCIL,
    PAINTBRUSH,
    COLOR_PICKER,
    PAINT_BUCKET,
    SPRAY_CAN,
};

class Editor : public QMainWindow
{
    Q_OBJECT

public:
    Editor(/*QWidget *parent = nullptr*/);
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

private:

    ImageWidget *imageWidget;
    QButtonGroup *toolGroup;
    Tool activeTool = PENCIL;

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
