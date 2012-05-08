##############

win32:INCLUDEPATH += D:/developing/ffmpeg/ffmpeg/include
win32:LIBS += -LD:/developing/ffmpeg/ffmpeg/lib

LIBS += \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale

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
