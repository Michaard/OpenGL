#define GLEW_STATIC
#define GLEW_NO_GLU
#include "GL/glew.h"
#include "GL/wglew.h"

#include <GL/glu.h>
#include <QApplication>
#include <QGLWidget>
#include <QTimer>
#include <QKeyEvent>
#include <cmath>
#include <iostream>

typedef GLfloat vec3f[3];

class Widget : public QGLWidget
{
    GLuint vbo;

    GLfloat xpos = 0;
    GLfloat ypos = 0;
    GLfloat zpos = 0;

    vec3f cameraPos={0,0,0};
    vec3f cameraFront={0,0,-1};
    vec3f cameraUp={0,1,0};

    vec3f crossProduct;
    vec3f normalVec;

    GLfloat yaw=0;
    GLfloat sunRotation=0;
    GLfloat* sunPosition;
    bool sunMode=0;

    int mapSizeX;
    int mapSizeY;
    int** mapYcoordinates;

    GLfloat mapXtrans=-512;
    GLfloat mapYtrans=-100;
    GLfloat mapZtrans=-100;

    QImage texture1;
    QImage texture2;
    QImage textureSkyboxXn;
    QImage textureSkyboxXp;
    QImage textureSkyboxYn;
    QImage textureSkyboxYp;
    QImage textureSkyboxZn;
    QImage textureSkyboxZp;

    GLuint texHandle[2];
    GLuint skyTex;

    void cross(vec3f a,vec3f b){
        GLfloat resx=a[1]*b[2]-a[2]*b[1];
        GLfloat resy=a[2]*b[0]-a[0]*b[2];
        GLfloat resz=a[0]*b[1]-a[1]*b[0];
        crossProduct[0]=resx;
        crossProduct[1]=resy;
        crossProduct[2]=resz;
    }

    void cross(GLfloat a,GLfloat b,GLfloat c,GLfloat d,GLfloat e,GLfloat f){
        GLfloat resx=b*f-c*e;
        GLfloat resy=c*d-a*f;
        GLfloat resz=a*e-b*d;
        crossProduct[0]=resx;
        crossProduct[1]=resy;
        crossProduct[2]=resz;
    }

    void normalize(vec3f v){
        GLfloat len=sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
        if(len!=0){
            GLfloat x=v[0]/len;
            GLfloat y=v[1]/len;
            GLfloat z=v[2]/len;
            normalVec[0]=x;
            normalVec[1]=y;
            normalVec[2]=z;
        }
        else{
            normalVec[0]=0;
            normalVec[1]=0;
            normalVec[2]=0;
        }
    }

    void normalize(GLfloat a,GLfloat b,GLfloat c){
        GLfloat len=sqrt(a*a+b*b+c*c);
        if(len!=0){
            GLfloat x=a/len;
            GLfloat y=b/len;
            GLfloat z=c/len;
            normalVec[0]=x;
            normalVec[1]=y;
            normalVec[2]=z;
        }
        else{
            normalVec[0]=0;
            normalVec[1]=0;
            normalVec[2]=0;
        }
    }

    void drawSquare(vec3f vertexes[],uint indexes[],GLfloat r, GLfloat g, GLfloat b){
        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vertexes);

        glColor3f(r,g,b);
        glDrawElements(GL_TRIANGLES,6, GL_UNSIGNED_INT,indexes);

        glDisableClientState(GL_VERTEX_ARRAY);
    }

    void drawGrid(GLfloat r, GLfloat g, GLfloat b){
        glColor3f(r,g,b);

        int mapX,mapY,x,z;
        for(mapX=0,z=0;mapX<mapSizeX-1;mapX++,z--){
            for(x=0,mapY=0;x<mapSizeX;x++,mapY++){
                glBegin(GL_LINES);
                    glVertex3f(x,mapYcoordinates[mapX][mapY],z);
                    glVertex3f(x,mapYcoordinates[mapX+1][mapY],z-1);
                glEnd();
            }
        }
        for(mapY=0,x=0;mapY<mapSizeY-1;mapY++,x++){
            mapX=0;
            for(z=0;z>-mapSizeY;z--){
                glBegin(GL_LINES);
                    glVertex3f(x,mapYcoordinates[mapX][mapY],z);
                    glVertex3f(x+1,mapYcoordinates[mapX][mapY+1],z);
                glEnd();
                mapX++;
            }
        }
    }

    void drawTerrain(GLfloat r, GLfloat g, GLfloat b){
        glColor3f(r,g,b);
        //glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,texHandle[1]);
        int mapX,mapY,x,z;
        for(mapX=0,z=0;mapX<mapSizeX-1;mapX++,z--){
            for(x=0,mapY=0;x<mapSizeX-1;x++,mapY++){
                glBegin(GL_TRIANGLE_STRIP);
                    cross(x,mapYcoordinates[mapX][mapY],z,x,mapYcoordinates[mapX+1][mapY],z-1);
                    normalize(crossProduct);
                    glNormal3f(normalVec[0],normalVec[1],normalVec[2]);

                    glTexCoord2f(0,0);
                    glVertex3f(x,mapYcoordinates[mapX][mapY],z);
                    glTexCoord2f(1,0);
                    glVertex3f(x,mapYcoordinates[mapX+1][mapY],z-1);
                    glTexCoord2f(0,1);
                    glVertex3f(x+1,mapYcoordinates[mapX][mapY+1],z);
                    glTexCoord2f(1,1);
                    glVertex3f(x+1,mapYcoordinates[mapX+1][mapY+1],z-1);
                glEnd();
            }
        }
        glBindTexture(GL_TEXTURE_2D,0);
        glDisable(GL_TEXTURE_2D);
    }

    void drawSky(){
        glColor3f(255,255,255);
        int s=2000;
        //glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_CUBE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP,skyTex);
        glTexCoord3f(0,0,0);
        glBegin(GL_QUADS); //xp
            glTexCoord3f(1,1,-1);
            glVertex3f(s,-s,-s);
            glTexCoord3f(1,-1,-1);
            glVertex3f(s,s,-s);
            glTexCoord3f(1,-1,1);
            glVertex3f(s,s,s);
            glTexCoord3f(1,1,1);
            glVertex3f(s,-s,s);
        glEnd();
        glBegin(GL_QUADS); //xn
            glTexCoord3f(-1,1,1);
            glVertex3f(-s,-s,s);
            glTexCoord3f(-1,-1,1);
            glVertex3f(-s,s,s);
            glTexCoord3f(-1,-1,-1);
            glVertex3f(-s,s,-s);
            glTexCoord3f(-1,1,-1);
            glVertex3f(-s,-s,-s);
        glEnd();
        glBegin(GL_QUADS); //yp
            glTexCoord3f(-1,1,1);
            glVertex3f(-s,s,-s);
            glTexCoord3f(-1,1,-1);
            glVertex3f(-s,s,s);
            glTexCoord3f(1,1,-1);
            glVertex3f(s,s,s);
            glTexCoord3f(1,1,1);
            glVertex3f(s,s,-s);
        glEnd();
        glBegin(GL_QUADS); //yn
            glTexCoord3f(-1,-1,1);
            glVertex3f(-s,-s,s);
            glTexCoord3f(-1,-1,-1);
            glVertex3f(-s,-s,-s);
            glTexCoord3f(1,-1,-1);
            glVertex3f(s,-s,-s);
            glTexCoord3f(1,-1,1);
            glVertex3f(s,-s,s);
        glEnd();
        glBegin(GL_QUADS); //zp
            glTexCoord3f(-1,1,-1);
            glVertex3f(-s,-s,-s);
            glTexCoord3f(-1,-1,-1);
            glVertex3f(-s,s,-s);
            glTexCoord3f(1,-1,-1);
            glVertex3f(s,s,-s);
            glTexCoord3f(1,1,-1);
            glVertex3f(s,-s,-s);
        glEnd();
        glBegin(GL_QUADS); //zn
            glTexCoord3f(1,1,1);
            glVertex3f(s,-s,s);
            glTexCoord3f(1,-1,1);
            glVertex3f(s,s,s);
            glTexCoord3f(-1,-1,1);
            glVertex3f(-s,s,s);
            glTexCoord3f(-1,1,1);
            glVertex3f(-s,-s,s);
        glEnd();
        glBindTexture(GL_TEXTURE_CUBE_MAP,0);
        glDisable(GL_TEXTURE_CUBE_MAP);
    }

    void drawTableLeg(GLfloat r, GLfloat g, GLfloat b){
        glColor3f(r,g,b);

        vec3f vertexes[4]=  {
                                {0,0,0},
                                {0,1,0},
                                {1,1,0},
                                {1,0,0}
                            };

        uint indexes[]={0, 1, 2, 0, 2, 3};

        glScalef(0.03,0.5,0.03);

        glPushMatrix();
        drawSquare(vertexes,indexes,r-0.2,g-0.2,b-0.2);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(1,0,0);
        glRotatef(90,0,1,0);
        drawSquare(vertexes,indexes,r-0.2,g-0.2,b-0.2);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,0,0);
        glRotatef(90,0,1,0);
        drawSquare(vertexes,indexes,r,g,b);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,0,-1);
        drawSquare(vertexes,indexes,r,g,b);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,0,0);
        glRotatef(-90,1,0,0);
        drawSquare(vertexes,indexes,0,0,0);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,1,0);
        glRotatef(-90,1,0,0);
        drawSquare(vertexes,indexes,0,0,0);
        glPopMatrix();
    }

    void drawTableDesktop(GLfloat r, GLfloat g, GLfloat b){
        glColor3f(r,g,b);

        vec3f vertexes[4]=  {
                                {0,0,0},
                                {0,1,0},
                                {1,1,0},
                                {1,0,0}
                            };

        uint indexes[]={0, 1, 2, 0, 2, 3};

        glScalef(1,0.04,0.8);

        glPushMatrix();
        drawSquare(vertexes,indexes,r-0.2,g-0.2,b-0.2);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(1,0,0);
        glRotatef(90,0,1,0);
        drawSquare(vertexes,indexes,r-0.2,g-0.2,b-0.2);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,0,0);
        glRotatef(90,0,1,0);
        drawSquare(vertexes,indexes,r,g,b);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,0,-1);
        drawSquare(vertexes,indexes,r,g,b);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,0,0);
        glRotatef(-90,1,0,0);
        drawSquare(vertexes,indexes,r,g,b);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,1,0);
        glRotatef(-90,1,0,0);
        drawSquare(vertexes,indexes,r+0.1,g+0.1,b+0.1);
        glPopMatrix();
    }

    void drawTable(GLfloat r, GLfloat g, GLfloat b){
        glColor3f(r,g,b);

        glPushMatrix();
        drawTableDesktop(r,g,b);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.1,-0.5,-0.1);
        drawTableLeg(0.8,0.8,0.8);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.1,-0.5,-0.65);
        drawTableLeg(0.8,0.8,0.8);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.88,-0.5,-0.1);
        drawTableLeg(0.8,0.8,0.8);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.88,-0.5,-0.65);
        drawTableLeg(0.8,0.8,0.8);
        glPopMatrix();
    }

    void drawPyramidh(){
        vec3f vertexes[5]=  {
                                {0,0,0},
                                {-1,-1,1},
                                {-1,-1,-1},
                                {1,-1,1},
                                {1,-1,-1}
                            };

        vec3f colors[5]=    {
                                {1,0,0},
                                {0,1,0},
                                {0,0,1},
                                {0,0,1},
                                {0,1,0}
                            };

        uint indexes[]={0,1,2, 0,1,3, 0,3,4, 0,2,4, 1,2,3, 2,3,4};

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vertexes);
        glColorPointer(3, GL_FLOAT, 0, colors);

        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT,indexes);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }

protected:
    void initializeGL()
    {
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            qDebug((const char*)glewGetErrorString(err));
        }
        else
        {
            vec3f verts[3] = { {0,1,0}, {-1,-1,0}, {1,-1,0} };
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            QImage greyMap=QImage("grey_map.jpg");
            mapSizeY=greyMap.width();
            mapSizeX=greyMap.height();
            mapYcoordinates=new int*[mapSizeX];
            for(int i=0;i<mapSizeX;i++){
                mapYcoordinates[i]=new int[mapSizeY];
            }
            for(int x=0;x<mapSizeX;x++){
                QRgb* point=(QRgb*)greyMap.scanLine(x);
                for(int y=0;y<mapSizeX;y++){
                    mapYcoordinates[x][y]=qGreen(point[y]);
                }
            }

            texture1=QImage("rock_texture.jpg");
            texture1=QGLWidget::convertToGLFormat(texture1);
            texture2=QImage("grass_texture.jpg");
            texture2=QGLWidget::convertToGLFormat(texture2);

            textureSkyboxXn=QImage("sky_xn.png");
            textureSkyboxXn=QGLWidget::convertToGLFormat(textureSkyboxXn);
            textureSkyboxXp=QImage("sky_xp.png");
            textureSkyboxXp=QGLWidget::convertToGLFormat(textureSkyboxXp);
            textureSkyboxYn=QImage("sky_yn.png");
            textureSkyboxYn=QGLWidget::convertToGLFormat(textureSkyboxYn);
            textureSkyboxYp=QImage("sky_yp.png");
            textureSkyboxYp=QGLWidget::convertToGLFormat(textureSkyboxYp);
            textureSkyboxZn=QImage("sky_zn.png");
            textureSkyboxZn=QGLWidget::convertToGLFormat(textureSkyboxZn);
            textureSkyboxZp=QImage("sky_zp.png");
            textureSkyboxZp=QGLWidget::convertToGLFormat(textureSkyboxZp);

            glGenTextures(1, &texHandle[0]);
                glBindTexture(GL_TEXTURE_2D, texHandle[0]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_LINEAR);
                    glTexImage2D(GL_TEXTURE_2D,0,4,texture1.width(),texture1.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texture1.bits());
                glBindTexture(GL_TEXTURE_2D,0);
                glBindTexture(GL_TEXTURE_2D, texHandle[1]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_LINEAR);
                    glTexImage2D(GL_TEXTURE_2D,0,4,texture2.width(),texture2.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texture2.bits());
                glBindTexture(GL_TEXTURE_2D,0);

            glGenTextures(1, &skyTex);
                glBindTexture(GL_TEXTURE_CUBE_MAP,skyTex);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,0,GL_RGBA8,textureSkyboxXp.width(),textureSkyboxXp.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,textureSkyboxXp.bits());
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X,0,GL_RGBA8,textureSkyboxXn.width(),textureSkyboxXn.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,textureSkyboxXn.bits());
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,0,GL_RGBA8,textureSkyboxYp.width(),textureSkyboxYp.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,textureSkyboxYp.bits());
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,0,GL_RGBA8,textureSkyboxYn.width(),textureSkyboxYn.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,textureSkyboxYn.bits());
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z,0,GL_RGBA8,textureSkyboxZp.width(),textureSkyboxZp.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,textureSkyboxZp.bits());
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,0,GL_RGBA8,textureSkyboxZn.width(),textureSkyboxZn.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,textureSkyboxZn.bits());
                glBindTexture(GL_TEXTURE_CUBE_MAP,0);

            glClearColor(0.2,0.2,0.2,1);
            glShadeModel(GL_SMOOTH);
            glClearDepth(1.0f);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
            glEnable(GL_LIGHTING);

            glEnable(GL_LIGHT0);
            float ambient[] = {0.2, 0.2, 0.2};
            float diffuse[] = {1, 1, 1};
            float specular[] = {1, 1, 1};
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
            GLfloat position[]={mapXtrans-300,mapYtrans,(-mapSizeY/2.0f)+mapZtrans};
            sunPosition=new GLfloat[3];
            sunPosition[0]=position[0];
            sunPosition[1]=position[1];
            sunPosition[2]=position[2];
            //std::cout << "Pozycja startowa: " << sunPosition[0] << " " << sunPosition[1] << " " << sunPosition[2] << std::endl;
            glLightfv(GL_LIGHT0, GL_POSITION, sunPosition);

            QTimer *timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(update()));
            timer->start(100);
        }
    }

    void paintGL()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-2.0,2.0,-2.0,2.0,5,-5.0);
        gluPerspective(20,1,1,100);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();

        glPushMatrix();
        glPushAttrib(GL_LIGHTING_BIT);
            float amb1[] = {1,1,1,1};
            glMaterialfv(GL_FRONT, GL_AMBIENT, amb1);
            float diff1[] = {0.1, 0.1, 0.1, 1};
            glMaterialfv(GL_FRONT, GL_DIFFUSE, diff1);
            float spec1[] = {0.01, 0.01, 0.01, 1};
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec1);

            glRotatef(yaw*40,0,1,0);
            drawSky();
        glPopAttrib();
        glPopMatrix();

        gluLookAt(cameraPos[0],cameraPos[1],cameraPos[2],cameraPos[0]+cameraFront[0],cameraPos[1]+cameraFront[1],cameraPos[2]+cameraFront[2],cameraUp[0],cameraUp[1],cameraUp[2]);
        glTranslatef(0, 0, -3.0);

        glPushMatrix();

        glPushAttrib(GL_LIGHTING_BIT);
            float amb[] = {1,1,1,1};
            glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
            float diff[] = {0.1, 0.1, 0.1, 1};
            glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
            float spec[] = {0.01, 0.01, 0.01, 1};
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
            //glMateriali(GL_FRONT, GL_SHININESS, 20);

            //glRotatef(45,0,1,0);
            glTranslatef(mapXtrans,mapYtrans,mapZtrans);
            drawTerrain(0,1,0);
            //drawGrid(0,0,0);
        glPopAttrib();
        glPopMatrix();

        glPushMatrix();
        if(sunMode==0){ // ruch slonca od minimalnej do maksymalnej
            if(sunPosition[0]==(-mapXtrans)+300){
                sunMode=1;
                sunPosition[0]--;
            }
            else{
                sunPosition[0]++;
            }
            if(sunPosition[0]<=0){
                sunPosition[1]+=0.5;
            }
            else{
                sunPosition[1]-=0.5;
            }
        }
        if(sunMode==1){ //Ruch slonca od maksymalnej do minimalnej wysokosci
            if(sunPosition[0]==mapXtrans-300){
                sunMode=0;
                sunPosition[0]++;
            }
            else{
                sunPosition[0]--;
            }
            if(sunPosition[0]<=0){
                sunPosition[1]+=0.5;
            }
            else{
                sunPosition[1]-=0.5;
            }
        }
        //std::cout << sunPosition[0] << " " << sunPosition[1] << std::endl;
        glLightfv(GL_LIGHT0, GL_POSITION, sunPosition);
        glPopMatrix();

        /*vec3f vertexes[4]=  {
                                {0,0,0},
                                {0,2,0},
                                {2,2,0},
                                {2,0,0}
                            };

        uint indexes[]={0, 1, 2, 0, 2, 3};

        glPushMatrix();
        glTranslatef(-1,-1,-2);
        drawSquare(vertexes,indexes,1,0,0); // ściana czerwona
        glPopMatrix();

        glPushMatrix();
        glTranslatef(1,-1,0);
        glRotatef(90,0,1,0);
        drawSquare(vertexes,indexes,0,0,1); // ściana niebieska
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-1,-1,0);
        glRotatef(-90,1,0,0);
        drawSquare(vertexes,indexes,0,1,0); // podłoga
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-0.5,-0.5,-0.5);
        drawTable(0.64,0.328,0.18);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0,0,-0.9);
        glScalef(0.15,0.5,0.15);
        drawPyramidh();
        glPopMatrix();*/

        glPopMatrix();
        glPopMatrix();

        /*if(sunRotation==359)
            sunRotation=0;
        else
            sunRotation+=1;*/
    }

    void resizeGL(int w, int h)
    {
        glViewport(0,0,w,h);
    }

    void keyPressEvent(QKeyEvent *event) {
        GLfloat cameraSpeed=1;
        vec3f front;
        switch (event->key()) {
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_F1:
            setWindowState(windowState() ^ Qt::WindowFullScreen);
            break;
        case Qt::Key_Left:
            cross(cameraFront,cameraUp);
            normalize(crossProduct);
            cameraPos[0]-=normalVec[0]*cameraSpeed;
            cameraPos[1]-=normalVec[1]*cameraSpeed;
            cameraPos[2]-=normalVec[2]*cameraSpeed;
            break;
        case Qt::Key_Right:
            cross(cameraFront,cameraUp);
            normalize(crossProduct);
            cameraPos[0]+=normalVec[0]*cameraSpeed;
            cameraPos[1]+=normalVec[1]*cameraSpeed;
            cameraPos[2]+=normalVec[2]*cameraSpeed;
            break;
        case Qt::Key_Up:
            cameraPos[1]+=1;
            break;
        case Qt::Key_Down:
            cameraPos[1]-=1;
            break;
        case Qt::Key_W:
            cameraPos[0]+=cameraSpeed*cameraFront[0];
            cameraPos[1]+=cameraSpeed*cameraFront[1];
            cameraPos[2]+=cameraSpeed*cameraFront[2];
            break;
        case Qt::Key_S:
            cameraPos[0]-=cameraSpeed*cameraFront[0];
            cameraPos[1]-=cameraSpeed*cameraFront[1];
            cameraPos[2]-=cameraSpeed*cameraFront[2];
            break;
        case Qt::Key_A:
            cross(cameraFront,cameraUp);
            normalize(crossProduct);
            cameraPos[0]-=normalVec[0]*cameraSpeed;
            cameraPos[1]-=normalVec[1]*cameraSpeed;
            cameraPos[2]-=normalVec[2]*cameraSpeed;
            break;
        case Qt::Key_D:
            cross(cameraFront,cameraUp);
            normalize(crossProduct);
            cameraPos[0]+=normalVec[0]*cameraSpeed;
            cameraPos[1]+=normalVec[1]*cameraSpeed;
            cameraPos[2]+=normalVec[2]*cameraSpeed;
            break;
        case Qt::Key_Q:
            yaw-=0.02;
            front[0]=sin(yaw);
            front[1]=cameraFront[1];
            front[2]=-cos(yaw);
            normalize(front);
            cameraFront[0]=front[0];
            cameraFront[1]=normalVec[1];
            cameraFront[2]=front[2];
            break;
        case Qt::Key_E:
            yaw+=0.02;
            front[0]=sin(yaw);
            front[1]=cameraFront[1];
            front[2]=-cos(yaw);
            normalize(front);
            cameraFront[0]=front[0];
            cameraFront[1]=normalVec[1];
            cameraFront[2]=front[2];
        default:
            break;
        }
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();
}
