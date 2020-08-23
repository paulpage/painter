#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <QPalette>
#include <QSizePolicy>
#include <QMenuBar>
#include <QKeySequence>
#include <QScrollBar>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QMessageBox>
#include <QGuiApplication>
#include <QStatusBar>

#include "Bitmap.h"
#include "ImageWidget.h"

class ImageEditor : public QMainWindow
{
    Q_OBJECT

public:
    ImageEditor(QWidget *parent = nullptr);
    ~ImageEditor();

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
