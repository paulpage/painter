#include "ImageEditor.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ImageEditor window;
    window.show();
    return app.exec();
}
