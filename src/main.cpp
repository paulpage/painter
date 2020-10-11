/* #include "phantomstyle.h" */
#include "Editor.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    /* QApplication::setStyle(new PhantomStyle); */
    Editor window;
    window.show();
    return app.exec();
}
