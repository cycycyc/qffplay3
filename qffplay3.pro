##############

win32{
    INCLUDEPATH += E:/developing/ffmpeg/ffmpeg/include
    LIBS += \
        -LE:/developing/ffmpeg/ffmpeg/lib \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lswscale
}

unix{
    INCLUDEPATH = /usr/local/include/
    LIBS = \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lswscale
}

CONFIG += static

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
