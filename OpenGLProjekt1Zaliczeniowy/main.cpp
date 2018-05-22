#include "imagewidget.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    ImageWidget iw;
    iw.show();
    return app.exec(); //w razie czego skasowac return
}
