QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32 {
    INCLUDEPATH += E:/developing/ffmpeg/ffmpeg/include
    LIBS += \
        -LE:/developing/ffmpeg/ffmpeg/lib \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lswscale
}

macx {
    INCLUDEPATH += /usr/local/include/
    LIBS += \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lswscale
}

unix:!mac {
    INCLUDEPATH += /usr/local/include/
    LIBS += \
        /usr/local/lib/libavcodec.a \
        /usr/local/lib/libavformat.a \
        /usr/local/lib/libavutil.a \
        /usr/local/lib/libswscale.a \
        /usr/local/lib/libx264.a
}

SOURCES += \
    main.cpp \
    decodethread.cpp \
    videothread.cpp \
    maindialog.cpp

HEADERS += \
    decodethread.h \
    ffmpeg.h \
    videothread.h \
    maindialog.h

DEFINES += __STDC_CONSTANT_MACROS

FORMS += \
    maindialog.ui
