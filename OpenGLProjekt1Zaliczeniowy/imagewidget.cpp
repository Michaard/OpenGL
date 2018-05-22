#include "imagewidget.h"
#include <cmath>
#include <QPainter>
#include <QKeyEvent>
#include <stack>
#include <iostream>

using namespace std;

ImageWidget::ImageWidget(QWidget *parent):QWidget(parent){
    image=QImage("sample.png");
    resultImage=image;
    brightness=0;
    QPoint p1(-1,-1);
    QPoint p2(-1,-1);
    mouse1=p1;
    mouse2=p2;
}

void ImageWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.drawImage(0,0,resultImage);
}

void ImageWidget::interpolationX(int prevX,int newX,int y){
    QRgb* point=(QRgb*)resultImage.scanLine(y);
    for(int x=prevX+1;x<newX;x++){
        point[x]=resultImage.pixel(prevX,y);
    }
}

void ImageWidget::interpolationY(int prevY,int newY){
    QRgb* point;
    int width=resultImage.width();
    for(int y=prevY+1;y<newY;y++){
        point=(QRgb*)resultImage.scanLine(y);
        for(int x=0;x<width;x++)
            point[x]=resultImage.pixel(x,prevY);
    }
}

void ImageWidget::processImage(){
    int height=image.height();
    int width=image.width();

    int prevY;
    int tmpy=-1;
    for(int y=0;y<height;y++){
        int prevX=-1;
        for(int x=0;x<width;x++){
            int* T1=translation(x,y,-256,-256);
            int* R=rotation(T1[0],T1[1],rotateAngle);
            int* Sc=scale(R[0],R[1],scalex,scaley);
            int* T2=translation(Sc[0],Sc[1],256,256);
            int* Sh=shear(T2[0],T2[1],shearValuex,shearValuey);
            int* newCord=translation(Sh[0],Sh[1],movex,movey);
            if(newCord[1]>=0 && newCord[1]<height){
                if(newCord[0]>=0 && newCord[0]<width){
                    tmpy=newCord[1];
                    QRgb* srcPicture=(QRgb*)image.scanLine(y);
                    QRgb* dstPicture=(QRgb*)resultImage.scanLine(newCord[1]);
                    int r=max(0,min(255,qRed(srcPicture[x])+brightness));
                    int g=max(0,min(255,qGreen(srcPicture[x])+brightness));
                    int b=max(0,min(255,qBlue(srcPicture[x])+brightness));
                    dstPicture[newCord[0]]=qRgba(r,g,b,qAlpha(srcPicture[x]));
                    if(x>0 && prevX!=-1){
                        int dif=abs(newCord[0]-prevX);
                        if(dif>1){
                            if(newCord[0]>prevX)
                                interpolationX(prevX,newCord[0],newCord[1]);
                            else
                                interpolationX(newCord[0],prevX,newCord[1]);
                        }
                    }
                    prevX=newCord[0];
                }
            }
            delete[]T1;
            delete[]T2;
            delete[]R;
            delete[]Sc;
            delete[]Sh;
            delete[]newCord;
        }
        if(y>0 && tmpy!=-1 && prevY!=-1){
            int dif=abs(tmpy-prevY);
            if(dif>1){
                if(tmpy>prevY)
                    interpolationY(prevY,tmpy);
                else
                    interpolationY(tmpy,prevY);
            }
        }
        prevY=tmpy;
    }
}

void ImageWidget::gradientHorizontal(QColor startColor,QColor endColor){
    bool clearColor=false;

    int r1=startColor.red();
    int g1=startColor.green();
    int b1=startColor.blue();
    int r2=endColor.red();
    int g2=endColor.green();
    int b2=endColor.blue();

    if(abs(r1-g1)<=10 || abs(r1-b1)<=10 || abs(g1-b1)<=10)
        clearColor=true;
    if(!clearColor && (abs(r2-g2)<=10 || abs(r2-b2)<=10 || abs(g2-b2)<=10))
        clearColor=true;

    int height=image.height();
    int width=image.width();

    int red=r1;
    int green=g1;
    int blue=b1;

    int changeFreqRed=0;
    int changeFreqGreen=0;
    int changeFreqBlue=0;

    int redDif;
    if(r1<=r2)
        redDif=r2-r1;
    else
        redDif=r1-r2;
    int greenDif;
    if(g1<=g2)
        greenDif=g2-g1;
    else
        greenDif=g1-g2;
    int blueDif;
    if(b1<=b2)
        blueDif=b2-b1;
    else
        blueDif=b1-b2;

    if(redDif!=0){
            changeFreqRed=width/redDif;
    }
    if(greenDif!=0){
            changeFreqGreen=width/greenDif;
    }
    if(blueDif!=0){
            changeFreqBlue=width/blueDif;
    }

    int dr=0;
    int dg=0;
    int db=0;

    if(r1<r2)
        dr=1;
    else if(r1>r2)
        dr=-1;
    if(g1<g2)
        dg=1;
    else if(g1>g2)
        dg=-1;
    if(b1<b2)
        db=1;
    else if(b1>b2)
        db=-1;

    for(int y=0;y<height;y++){
        int redStep=0;
        int greenStep=0;
        int blueStep=0;
        QRgb* dstPicture=(QRgb*)image.scanLine(y);
        for(int x=0;x<width;x++){
            if(redStep==changeFreqRed && changeFreqRed!=0 && red!=r2){
                if(clearColor){
                    if(!(green<g2-r2 || blue<b2-r2))
                        red+=dr;
                }
                else
                    red+=dr;
                redStep=0;
            }
            if(greenStep==changeFreqGreen && changeFreqGreen!=0 && green!=g2){
                if(clearColor){
                    if(!(red<r2-g2 || blue<b2-g2))
                        green+=dg;
                }
                else
                    green+=dg;
                greenStep=0;
            }
            if(blueStep==changeFreqBlue && changeFreqBlue!=0 && blue!=b2){
                if(clearColor){
                    if(!(red<r2-b2 || green<g2-b2))
                        blue+=db;
                }
                else
                    blue+=db;
                blueStep=0;
            }
            dstPicture[x]=qRgba(red,green,blue,qAlpha(dstPicture[x]));
            redStep++;
            greenStep++;
            blueStep++;
        }
        red=r1;
        green=g1;
        blue=b1;
    }
}

void ImageWidget::gradientVertical(QColor startColor,QColor endColor){
    bool clearColor=false;

    int r1=startColor.red();
    int g1=startColor.green();
    int b1=startColor.blue();
    int r2=endColor.red();
    int g2=endColor.green();
    int b2=endColor.blue();

    if(abs(r1-g1)<=10 || abs(r1-b1)<=10 || abs(g1-b1)<=10)
        clearColor=true;
    if(!clearColor && (abs(r2-g2)<=10 || abs(r2-b2)<=10 || abs(g2-b2)<=10))
        clearColor=true;

    int height=image.height();
    int width=image.width();

    int red=r1;
    int green=g1;
    int blue=b1;

    int changeFreqRed=0;
    int changeFreqGreen=0;
    int changeFreqBlue=0;

    int redDif;
    if(r1<=r2)
        redDif=r2-r1;
    else
        redDif=r1-r2;
    int greenDif;
    if(g1<=g2)
        greenDif=g2-g1;
    else
        greenDif=g1-g2;
    int blueDif;
    if(b1<=b2)
        blueDif=b2-b1;
    else
        blueDif=b1-b2;

    if(redDif!=0){
        changeFreqRed=width/redDif;
    }
    if(greenDif!=0){
        changeFreqGreen=width/greenDif;
    }
    if(blueDif!=0){
        changeFreqBlue=width/blueDif;
    }

    int dr=0;
    int dg=0;
    int db=0;

    if(r1<r2)
        dr=1;
    else if(r1>r2)
        dr=-1;
    if(g1<g2)
        dg=1;
    else if(g1>g2)
        dg=-1;
    if(b1<b2)
        db=1;
    else if(b1>b2)
        db=-1;

    int redStep=0;
    int greenStep=0;
    int blueStep=0;

    for(int y=0;y<height;y++){
        QRgb* dstPicture=(QRgb*)image.scanLine(y);
        if(redStep==changeFreqRed && changeFreqRed!=0 && red!=r2){
            if(clearColor){
                if(!(green<g2-r2 || blue<b2-r2))
                    red+=dr;
            }
            else
                red+=dr;
            redStep=0;
        }
        if(greenStep==changeFreqGreen && changeFreqGreen!=0 && green!=g2){
            if(clearColor){
                if(!(red<r2-g2 || blue<b2-g2))
                    green+=dg;
            }
            else
                green+=dg;
            greenStep=0;
        }
        if(blueStep==changeFreqBlue && changeFreqBlue!=0 && blue!=b2){
            if(clearColor){
                if(!(red<r2-b2 || green<g2-b2))
                    blue+=db;
            }
            else
                blue+=db;
            blueStep=0;
        }
        for(int x=0;x<width;x++){
            dstPicture[x]=qRgba(red,green,blue,qAlpha(dstPicture[x]));
        }
        redStep++;
        greenStep++;
        blueStep++;
    }
}

void ImageWidget::paintLine(QPoint startPoint,QPoint endPoint){
    int x1=startPoint.x();
    int y1=startPoint.y();
    int x2=endPoint.x();
    int y2=endPoint.y();

    int dx=x2-x1;
    int dy=y2-y1;

    int dx1,dx2,dy1;
    int dy2=0;

    if(dx<0){
        dx1=-1;
        dx2=-1;
    }
    else if(dx>0){
        dx1=1;
        dx2=1;
    }
    else{
        dx1=0;
        dx2=0;
    }

    if(dy<0){
        dy1=-1;
    }
    else if(dy>0){
        dy1=1;
    }
    else{
        dy1=0;
    }

    int w=abs(dx);
    int h=abs(dy);
    if(!(w>h)){
        int tmp=w;
        w=h;
        h=tmp;
        if(dy<0){
            dy2=-1;
        }
        else if(dy>0){
            dy2=1;
        }
        dx2=0;
    }
    int x=x1;
    int y=y1;
    int n=w;
    QRgb* point;

    for(int i=0;i<=w;i++){
        point=(QRgb*)image.scanLine(y);
        point[x]=qRgb(0,0,0);
        n+=h;
        if(!(n<w)){
            n-=w;
            x+=dx1;
            y+=dy1;
        }
        else{
            x+=dx2;
            y+=dy2;
        }
    }
}

void ImageWidget::paintCircle(QPoint middle,int r){
    int x0=middle.x();
    int y0=middle.y();

    int height=image.height();
    int width=image.width();

    int x=r;
    int y=0;
    int d=0;

    QRgb* point;
    while(x>=y){
        if(y0+y<height){
            point=(QRgb*)image.scanLine(y0+y);
            if(x0+x<width)
                point[x0+x]=qRgb(0,0,0);
            if(x0-x>=0)
                point[x0-x]=qRgb(0,0,0);
        }

        if(y0+x<height){
            point=(QRgb*)image.scanLine(y0+x);
            if(x0+y<width)
                point[x0+y]=qRgb(0,0,0);
            if(x0-y>=0)
                point[x0-y]=qRgb(0,0,0);
        }

        if(y0-y>=0){
            point=(QRgb*)image.scanLine(y0-y);
            if(x0+x<width)
                point[x0+x]=qRgb(0,0,0);
            if(x0-x>=0)
                point[x0-x]=qRgb(0,0,0);
        }

        if(y0-x>=0){
            point=(QRgb*)image.scanLine(y0-x);
            if(x0+y<width)
                point[x0+y]=qRgb(0,0,0);
            if(x0-y>=0)
                point[x0-y]=qRgb(0,0,0);
        }

        if(d<=0){
            y++;
            d+=2*y+1;
        }
        if(d>0){
            x--;
            d-=2*x+1;
        }
    }
}

QPoint getPointCL(QPoint points[],int r,int i,double t){
    if(r==0)
        return points[i];
    else{
        QPoint p1=getPointCL(points,r-1,i,t);
        QPoint p2=getPointCL(points,r-1,i+1,t);

        int x=(int)((1-t)*p1.x()+t*p2.x());
        int y=(int)((1-t)*p1.y()+t*p2.y());

        QPoint retPoint(x,y);
        return retPoint;
    }
}

void ImageWidget::paintCurvedLine(QPoint* points,int noPoints){
    QPoint tmp;
    QPoint prevPoint;
    QRgb* point;
    for(double t=0;t<=1;t+=0.1){
        tmp=getPointCL(points,noPoints-1,0,t);
        if(t==0){
            point=(QRgb*)image.scanLine(tmp.y());
            point[tmp.x()]=qRgb(0,0,0);
        }
        else{
            paintLine(prevPoint,tmp);
        }
        prevPoint=tmp;
    }
}

void ImageWidget::fillArea(QPoint startPoint,QColor wantedColor){
    int r=wantedColor.red();
    int g=wantedColor.green();
    int b=wantedColor.blue();

    int height=image.height();
    int width=image.width();

    int x=startPoint.x();
    int y=startPoint.y();
    QColor currentColor=image.pixelColor(x,y);
    stack<QPoint> pointStack;
    QPoint point;
    QRgb* colorPoint;

    pointStack.push(startPoint);

    while(!pointStack.empty()){
        point=pointStack.top();
        pointStack.pop();
        int newX=point.x();
        int newY=point.y();
        if(image.pixelColor(newX,newY)==currentColor && currentColor!=wantedColor){
            colorPoint=(QRgb*)image.scanLine(newY);
            colorPoint[newX]=qRgb(r,g,b);
            if(newY!=0){
                if(image.pixelColor(newX,newY-1)==currentColor){
                    QPoint pointUp(newX,newY-1);
                    pointStack.push(pointUp);
                }
            }
            if(newY!=height-1){
                if(image.pixelColor(newX,newY+1)==currentColor){
                    QPoint pointDown(newX,newY+1);
                    pointStack.push(pointDown);
                }
            }
            if(newX!=0){
                if(image.pixelColor(newX-1,newY)==currentColor){
                    QPoint pointLeft(newX-1,newY);
                    pointStack.push(pointLeft);
                }
            }
            if(newX!=width-1){
                if(image.pixelColor(newX+1,newY)==currentColor){
                    QPoint pointRight(newX+1,newY);
                    pointStack.push(pointRight);
                }
            }
        }
    }
}

int* ImageWidget::vectorXmatrix(int v[3],int m[9]){
    int* result=new int[3];
    result[0]=v[0]*m[0]+v[1]*m[3]+v[2]*m[6];
    result[1]=v[0]*m[1]+v[1]*m[4]+v[2]*m[7];
    result[2]=v[0]*m[2]+v[1]*m[5]+v[2]*m[8];

    return result;
}

int* ImageWidget::vectorXmatrix2(int v[3],int m[9]){
    int* result=new int[3];
    result[0]=(v[0]*m[0]+v[1]*m[3]+v[2]*m[6])>>8;
    result[1]=(v[0]*m[1]+v[1]*m[4]+v[2]*m[7])>>8;
    result[2]=(v[0]*m[2]+v[1]*m[5]+v[2]*m[8])>>8;

    return result;
}

int* ImageWidget::vectorXmatrix3(int v[3],double m[9]){
    int* result=new int[3];
    result[0]=(v[0]*m[0]+v[1]*m[3]+v[2]*m[6]);
    result[1]=(v[0]*m[1]+v[1]*m[4]+v[2]*m[7]);
    result[2]=(v[0]*m[2]+v[1]*m[5]+v[2]*m[8]);

    return result;
}

int* ImageWidget::translation(int x,int y,int tx,int ty){
    int transMatrix[9]={1,0,0,
                        0,1,0,
                        tx,ty,1};
    int v[3]={x,y,1};
    return vectorXmatrix(v,transMatrix);
}

int* ImageWidget::rotation(int x,int y,double angle){
    int s=sin(angle)*256;
    int c=cos(angle)*256;

    int rotMatrix[9]={c,s,0,
                      -s,c,0,
                      0,0,1};
    int v[3]={x,y,1};
    return vectorXmatrix2(v,rotMatrix);
}

int* ImageWidget::scale(int x,int y,double sx,double sy){
    int sxi=sx*256;
    int syi=sy*256;

    int scaleMatrix[9]={sxi,0,0,
                        0,syi,0,
                        0,0,1};
    int v[3]={x,y,1};
    return vectorXmatrix2(v,scaleMatrix);
}

int* ImageWidget::shear(int x,int y,double ax,double ay){
    double shearMatrix[9]={1,ay,0,
                        ax,1,0,
                        0,0,1};
    int v[3]={x,y,1};
    return vectorXmatrix3(v,shearMatrix);
}

void ImageWidget::mousePressEvent(QMouseEvent* e){
    if(e->button()==Qt::LeftButton){
        if(mouseClickCounter==0){
            mouse1=e->pos();
            mouseClickCounter++;
        }
        else if(mouseClickCounter==1){
            mouse2=e->pos();
            mouseClickCounter=0;
        }
        circleSize++;
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* e){
    if(e->button()==Qt::LeftButton){
        if(mouseClickCounter==1){
            int xDif=abs(e->x()-mouse1.x());
            int yDif=abs(e->y()-mouse1.y());
            if(xDif>yDif)
                circleSize=xDif;
            else
                circleSize=yDif;
        }
        else{
            int xDif=abs(e->x()-mouse2.x());
            int yDif=abs(e->y()-mouse2.y());
            if(xDif>yDif)
                circleSize=xDif;
            else
                circleSize=yDif;
        }
    }
}

void ImageWidget::keyPressEvent(QKeyEvent* e){
    if(e->key()==Qt::Key_Period)
        brightness+=10;
    if(e->key()==Qt::Key_Comma)
        brightness-=10;
    if(e->key()==Qt::Key_N){
        QColor color(255,255,255);
        image.fill(color);
        brightness=0;
        movex=0;
        movey=0;
        rotateAngle=0;
        scalex=1;
        scaley=1;
        shearValuex=0;
        shearValuey=0;
    }
    if(e->key()==Qt::Key_1){
        QColor c1(221,239,57);
        QColor c2(21,187,220);
        brightness=0;
        gradientHorizontal(c1,c2);
    }
    if(e->key()==Qt::Key_2){
        QColor c1(21,187,220);
        QColor c2(221,239,57);
        brightness=0;
        gradientVertical(c1,c2);
    }
    if(e->key()==Qt::Key_A){
        if(mouse1.x()!=-1 && mouse2.x()!=-1)
            paintLine(mouse1,mouse2);
    }
    if(e->key()==Qt::Key_C){
        if(mouse1.x()!=-1){
            if(mouseClickCounter==1)
                paintCircle(mouse1,circleSize);
            else
                paintCircle(mouse2,circleSize);
        }
        circleSize=0;
    }
    if(e->key()==Qt::Key_S){
        int size=4;
        QPoint* points=new QPoint[size];
        QPoint p0(100,100);
        QPoint p1(20,200);
        QPoint p2(200,100);
        QPoint p3(150,400);
        points[0]=p0;
        points[1]=p1;
        points[2]=p2;
        points[3]=p3;
        paintCurvedLine(points,size);
    }
    if(e->key()==Qt::Key_F){
        if(mouse1.x()!=-1){
            QColor c(0,0,255);
            if(mouseClickCounter==1)
                fillArea(mouse1,c);
            else
                fillArea(mouse2,c);
        }
    }
    if(e->key()==Qt::Key_Up){
        movey-=16;
    }
    if(e->key()==Qt::Key_Down){
        movey+=16;
    }
    if(e->key()==Qt::Key_Left){
        movex-=16;
    }
    if(e->key()==Qt::Key_Right){
        movex+=16;
    }
    if(e->key()==Qt::Key_L){
        rotateAngle+=M_PI/4;
    }
    if(e->key()==Qt::Key_R){
        rotateAngle-=M_PI/4;
    }
    if(e->key()==Qt::Key_Plus){
        scalex+=0.1;
        scaley+=0.1;
    }
    if(e->key()==Qt::Key_Minus){
        scalex-=0.1;
        scaley-=0.1;
    }
    if(e->key()==Qt::Key_Slash){
        shearValuex-=0.1;
        //shearValuey-=1;
    }
    if(e->key()==Qt::Key_Backslash){
        shearValuex+=0.1;
        //shearValuey+=1;
    }
    processImage();
    update();
}
