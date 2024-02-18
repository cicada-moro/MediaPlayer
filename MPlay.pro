QT      += core gui
QT      += core gui opengl widgets openglwidgets
QT      += multimedia
QT      += concurrent


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/audio_resample.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mediadecode.cpp \
    src/mediademux.cpp \
    src/mythread/myaudiothread.cpp \
    src/mythread/myplaythread.cpp \
    src/mythread/myvideothread.cpp \
    src/sonic/sonic.c \
    src/video_swsscale.cpp \
    src/widget/myaudioplay.cpp \
    src/widget/myborder.cpp \
    src/widget/mybordercontainer.cpp \
    src/widget/myopenglwidget.cpp \
    src/widget/myslider.cpp \
    src/widget/qtaudioplay.cpp \
    src/widget/sdlaudioplay.cpp

HEADERS += \
    src/audio_resample.h \
    src/mainwindow.h \
    src/mediadecode.h \
    src/mediademux.h \
    src/mythread/myaudiothread.h \
    src/mythread/myplaythread.h \
    src/mythread/myvideothread.h \
    src/sonic/sonic.h \
    src/video_swsscale.h \
    src/widget/myaudioplay.h \
    src/widget/mybordercontainer.h \
    src/widget/myopenglwidget.h \
    src/widget/myslider.h \
    src/widget/qtaudioplay.h \
    src/widget/sdlaudioplay.h

FORMS += \
    src/mainwindow.ui

INCLUDEPATH += \
    $$PWD/include

LIBS += -L$$PWD/lib\
    -lavcodec \
    -lavdevice \
    -lavfilter \
    -lavformat \
    -lavutil \
    -lpostproc \
    -lswresample \
    -lswscale \
    -lSDL2 \
    -lSDL2main \
    -lSDL2test

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res/res.qrc \

DISTFILES += \
    fragment.fsh \
    res/image/Bold.png \
    res/image/CH.png \
    res/image/Copy.png \
    res/image/Cut.png \
    res/image/EH.png \
    res/image/Italic.png \
    res/image/New.png \
    res/image/OIP-C.gif \
    res/image/Open.png \
    res/image/Paste.png \
    res/image/Redo.png \
    res/image/Save.png \
    res/image/Tabs.png \
    res/image/Underline.png \
    res/image/Undo.png \
    res/image/before.png \
    res/image/close.png \
    res/image/flush.png \
    res/image/loading.gif \
    res/image/mute.png \
    res/image/next.png \
    res/image/page.png \
    res/image/setfont.png \
    res/image/setting.png \
    res/image/small_size.png \
    res/image/start.png \
    res/image/stop.png \
    res/image/tiled.png \
    res/image/volume.png \
    res/image/window_enlarged.png \
    res/image/window_smalled.png \
    res/opengl/fragment.fsh \
    res/opengl/fragment_yuv.fsh \
    res/opengl/vertex.vsh \
    res/opengl/vertex_yuv.vsh \
    res/qss/widget.qss \
    vertex.vsh
