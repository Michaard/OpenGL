#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>

class ImageWidget : public QWidget{
private:
    QPoint mouse1;
    QPoint mouse2;
    int mouseClickCounter=0;
    int circleSize=0;
    QImage image;
    QImage resultImage;
    int brightness;
    int movex=0;
    int movey=0;
    double rotateAngle=0;
    double scalex=1;
    double scaley=1;
    double shearValuex=0;
    double shearValuey=0;
    void processImage();
protected:
    void gradientHorizontal(QColor,QColor);
    void gradientVertical(QColor,QColor);
    void paintLine(QPoint,QPoint);
    void paintCircle(QPoint,int);
    void paintCurvedLine(QPoint*,int);
    void fillArea(QPoint,QColor);
    int* vectorXmatrix(int*,int*);
    int* vectorXmatrix2(int*,int*);
    int* vectorXmatrix3(int*,double*);
    int* translation(int,int,int,int);
    int* rotation(int,int,double);
    int* scale(int,int,double,double);
    int* shear(int,int,double,double);
    void interpolationX(int,int,int);
    void interpolationY(int,int);
    virtual void keyPressEvent(QKeyEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void paintEvent(QPaintEvent*);
public:
    ImageWidget(QWidget *parent=0);
};

#endif // IMAGEWIDGET_H
