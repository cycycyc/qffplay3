##############

INCLUDEPATH += D:/developing/ffmpeg/ffmpeg/include

LIBS += \
    -LD:/developing/ffmpeg/ffmpeg/lib \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale

SOURCES += \
    main.cpp \
    decodethread.cpp \
    videothread.cpp \
    screen.cpp \
    maindialog.cpp

HEADERS += \
    decodethread.h \
    ffmpeg.h \
    videothread.h \
    screen.h \
    maindialog.h

DEFINES += __STDC_CONSTANT_MACROS

FORMS += \
    maindialog.ui
