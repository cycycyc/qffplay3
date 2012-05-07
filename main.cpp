#include <QApplication>
#include "maindialog.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    //DecodeThread d;
    //d.openFile("http://v.youku.com/player/getRealM3U8/vid/XMzgyNjUyMjMy/type//video.m3u8");
    //d.openFile("http://v.youku.com/player/getRealM3U8/vid/XMzkwNDI0OTI0/type//video.m3u8");
    //d.openFile("http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8");
    //d.start();
    MainDialog d;
    d.show();
    return app.exec();
}
