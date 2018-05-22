QT += opengl

SOURCES += \
    main.cpp \
    glew.c

HEADERS += \
    GL/glew.h \
    GL/wglew.h

LIBS += -L"D:\Materialy do zajec\Informatyka\OpenGL\glew-2.0.0\lib\Release\Win32" -lglew32 -lglu32 -lOpengl32
